#include "Curler.hpp"
#include <fstream>
#include <sstream>
#include <cppcodec/base64_rfc4648.hpp>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <iostream>
#include <filesystem>
#include "DateTime.hpp"
#include "Resources.hpp"
#include <iomanip>
#include <ctime>
#include <QUrl>
using json = nlohmann::json;
namespace fs = std::filesystem;

namespace Curler
{
    HttpResponse performRequest(const std::string &url,
                                const std::optional<std::string> &payload,
                                HttpMethod method,
                                const std::optional<std::string> &certPath,
                                const std::optional<std::string> &keyPath)
    {
        CURL *curl = curl_easy_init();
        if (!curl)
            throw std::runtime_error("Error initializing cURL.");

        std::unique_ptr<std::string> responseData(new std::string());
        struct curl_slist *headers = nullptr;

        headers = curl_slist_append(headers, "Accept: */*");
        headers = curl_slist_append(headers, "Content-Type: application/json");

        NetLogger::IoLog iolog;
        auto cb_bundle = std::make_pair(responseData.get(), &iolog);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NetLogger::logging_write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cb_bundle);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, NetLogger::curl_debug_cb);
        curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &iolog);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.11.1-DEV");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        if (certPath && keyPath)
        {
            curl_easy_setopt(curl, CURLOPT_SSLCERT, certPath->c_str());
            curl_easy_setopt(curl, CURLOPT_SSLKEY, keyPath->c_str());
        }

        size_t request_payload_size = 0;
        if (method == HttpMethod::HTTP_POST || method == HttpMethod::HTTP_DELETE)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload->c_str());
            request_payload_size = payload ? payload->size() : 0;
            if (method == HttpMethod::HTTP_POST)
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
            else
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }
        else
        {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }

        // Timestamp for log
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm{};
#ifdef _WIN32
        localtime_s(&local_tm, &now_c);
#else
        localtime_r(&now_c, &local_tm);
#endif

        // Capture request start in steady clock microseconds
        auto req_start_tp = std::chrono::steady_clock::now();
        auto req_start_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                req_start_tp.time_since_epoch())
                                .count();

        CURLcode res = curl_easy_perform(curl);

        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        long num_connects = 0;
        curl_easy_getinfo(curl, CURLINFO_NUM_CONNECTS, &num_connects);

        char *primary_ip = nullptr;
        curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &primary_ip);
        // Get download size from libcurl
        curl_off_t dl_size_bytes = 0;
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &dl_size_bytes);

        // libcurl timings
        double t_namelookup = 0, t_connect = 0, t_appconnect = 0, t_pretransfer = 0,
               t_starttransfer = 0, t_total = 0, t_redirect = 0;
        curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &t_namelookup);
        curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &t_connect);
        curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &t_appconnect);
        curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &t_pretransfer);
        curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &t_starttransfer);
        curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME, &t_redirect);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &t_total);

        uint64_t tx_us = iolog.first_byte_out_us.load();
        uint64_t rx_us = iolog.first_byte_in_us.load();

        double tx_ms_rel = (tx_us > req_start_us)
                               ? (tx_us - req_start_us) / 1000.0
                               : -1.0;

        double rx_ms_rel = (rx_us > req_start_us)
                               ? (rx_us - req_start_us) / 1000.0
                               : -1.0;

        // Our network-only TTFB in ms
        double ttfb_network_only_ms = (tx_us > 0 && rx_us > tx_us)
                                          ? (rx_us - tx_us) / 1000.0
                                          : -1.0;

        double dl_duration_ms = (t_total - t_starttransfer) * 1000.0;
        // Write to log file
        {
            std::filesystem::create_directories("netlogs");
            QUrl wholeURL(url.c_str());

            std::string host = wholeURL.host().toStdString();
            if (host.empty())
                host = "unknown";
            std::replace_if(host.begin(), host.end(), [](char c)
                            { return !(std::isalnum((unsigned char)c) || c == '.' || c == '-'); }, '_');

            std::string urlpath = wholeURL.path().toStdString();

            if (urlpath.empty())
                urlpath = "unknown";
            std::replace_if(urlpath.begin(), urlpath.end(), [](char c)
                            { return !(std::isalnum((unsigned char)c) || c == '.' || c == '-'); }, '_');

            std::string logName = host + urlpath + ".log";
            std::ofstream log("netlogs/" + logName, std::ios::app);
            // LOGGING DISABLED
            if (log && false)
            {
                const char *methodStr =
                    (method == HttpMethod::HTTP_GET) ? "GET" : (method == HttpMethod::HTTP_POST) ? "POST"
                                                           : (method == HttpMethod::HTTP_DELETE) ? "DELETE"
                                                                                                 : "OTHER";

                log << "=== Request Log Entry ===\n";
                log << "Timestamp: " << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "\n";
                log << "URL: " << url << "\n";
                log << "Method: " << methodStr << "\n";
                log << "Number of connects so far: " << num_connects << "\n";
                if (primary_ip)
                    log << "Server IP: " << primary_ip << "\n";
                log << "Request payload size: " << request_payload_size << " bytes\n";
                log << "Response payload size (string): " << responseData->size() << " bytes\n";
                log << "Response payload size (CURL): " << dl_size_bytes << " bytes\n";
                log << "HTTP Code: " << httpCode << "\n";
                log << "First TX byte: " << tx_ms_rel << " ms after request start\n";
                log << "First RX byte: " << rx_ms_rel << " ms after request start\n";
                log << "Download duration: " << dl_duration_ms << " ms\n";

                if (ttfb_network_only_ms >= 0)
                    log << "TTFB (network only): " << ttfb_network_only_ms << " ms\n";
                log << "TTFB (full, libcurl): " << (t_starttransfer * 1000.0) << " ms\n";

                log << "--- libcurl timing (ms) ---\n";
                log << "DNS lookup:    " << (t_namelookup * 1000.0) << "\n";
                log << "TCP connect:   " << ((t_connect - t_namelookup) * 1000.0) << "\n";
                log << "TLS handshake: " << ((t_appconnect - t_connect) * 1000.0) << "\n";
                log << "Pre-transfer:  " << (t_pretransfer * 1000.0) << "\n";
                log << "Start-transfer:" << (t_starttransfer * 1000.0) << "\n";
                log << "Redirect:      " << (t_redirect * 1000.0) << "\n";
                log << "Total:         " << (t_total * 1000.0) << "\n";
                log << "--------------------------------------\n\n";
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
            throw std::runtime_error("cURL request failed: " + std::string(curl_easy_strerror(res)));

        return HttpResponse{*responseData, httpCode};
    }

    HttpResponse getJson(const std::string &url,
                         const std::optional<std::string> &certPath,
                         const std::optional<std::string> &keyPath)
    {
        return performRequest(url, std::nullopt, HttpMethod::HTTP_GET, certPath, keyPath);
    }

    HttpResponse postJson(const std::string &url,
                          const std::string &jsonPayload,
                          const std::optional<std::string> &certPath,
                          const std::optional<std::string> &keyPath)
    {
        return performRequest(url, jsonPayload, HttpMethod::HTTP_POST, certPath, keyPath);
    }

    HttpResponse deleteJson(const std::string &url,
                            const std::string &jsonPayload,
                            const std::optional<std::string> &certPath,
                            const std::optional<std::string> &keyPath)
    {
        return performRequest(url, jsonPayload, HttpMethod::HTTP_DELETE, certPath, keyPath);
    }

    static std::size_t callback(
        const char *in,
        std::size_t size,
        std::size_t num,
        std::string *out)
    {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }

    std::map<std::string, int> getAllAckTypes()
    {
        std::string url = ConfigManager::getInstance().getConfig().api_endpoint.value() + "/v1/ack/getAllTypes";
        auto response = getJson(url);
        if (response.body.empty())
        {
            std::cerr << "Error: Empty response received." << std::endl;
            return {};
        }

        std::map<std::string, int> ackTypes;

        try
        {
            auto jsonResponse = nlohmann::json::parse(response.body);

            for (auto &[key, value] : jsonResponse.items())
            {
                if (value.is_number_integer()) // Ensure it's an integer type
                {
                    ackTypes[key] = value.get<int>();
                }
                else
                {
                    std::cerr << "Invalid type for UUID: " << key << std::endl;
                }
            }
        }
        catch (const nlohmann::json::parse_error &e)
        {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
        }
        catch (const nlohmann::json::type_error &e)
        {
            std::cerr << "JSON type error: " << e.what() << std::endl;
        }

        return ackTypes;
    }

    // Function to extract gml:id from XML
    std::string extractGmlId(const std::string &xmlData)
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xmlData.c_str());

        if (!result)
        {
            std::cerr << "XML Parsing error: " << result.description() << std::endl;
            return "";
        }

        // Find the Dataset element
        //pugi::xml_node dataset = doc.child("Dataset");
        pugi::xml_node dataset = doc.select_node("//*[local-name()='Dataset']").node();

        if (!dataset)
        {
            std::cerr << "Dataset element not found in XML." << std::endl;
            return "";
        }

        // Get the gml:id attribute
        pugi::xml_attribute idAttr = dataset.attribute("gml:id");
        if (!idAttr)
        {
            std::cerr << "gml:id attribute not found in XML." << std::endl;
            return "";
        }

        return idAttr.value();
    }

    // Move to datalayer
    void saveAndDecodeRawJSONToFiles(const std::string jsonInput, const std::string outputFolder)
    {
        // Parse JSON
        json jsonData;
        try
        {
            jsonData = json::parse(jsonInput);
        }
        catch (const json::parse_error &e)
        {
            std::cerr << jsonInput << '\n';
            throw std::runtime_error("Failed to parse JSON response: " + std::string(e.what()));
        }

        // Ensure response contains expected structure
        if (!jsonData.contains("dataResponseObject") || !jsonData["dataResponseObject"].is_array())
        {
            throw std::runtime_error("JSON response does not contain 'dataResponseObject' array.");
        }

        // Process and save Base64-decoded XML data
        int index = 0;
        for (const auto &item : jsonData["dataResponseObject"])
        {
            if (!item.contains("data") || !item["data"].is_string())
            {
                continue;
            }

            std::string base64Data = item["data"];
            std::string decodedData = cppcodec::base64_rfc4648::decode<std::string>(base64Data);

            // Extract gml:id from XML
            std::string gmlId = extractGmlId(decodedData);
            std::string outputFilename;

            if (!gmlId.empty())
            {
                outputFilename = outputFolder + gmlId;
            }
            else
            {
                outputFilename = outputFolder + "output_" + std::to_string(index);
            }

            outputFilename += ".xml";
            // Save decoded data to file
            std::ofstream outFile(outputFilename);
            if (!outFile)
            {
                throw std::runtime_error("Failed to open output file: " + outputFilename);
            }
            outFile << decodedData;
            outFile.close();

            //std::cout << "Saved decoded XML to " << outputFilename << std::endl;
            ++index;
        }
    }

    std::vector<std::string> getFileList()
    {
        std::vector<std::string> fileList;

        try
        {
            const std::string url = ConfigManager::getInstance().getConfig().api_endpoint.value() + "/v1/files";
            auto response = getJson(url);
            if (response.body.empty())
                return fileList;

            auto jsonResponse = json::parse(response.body, nullptr, false);
            if (jsonResponse.is_discarded() || !jsonResponse.is_array())
                return fileList;

            for (const auto &item : jsonResponse)
            {
                if (item.is_string())
                {
                    fileList.push_back(item.get<std::string>());
                }
            }
        }
        catch (const std::exception &e)
        {
            // Log error if needed
            std::cerr << "Error fetching file list: " << e.what() << std::endl;
        }

        return fileList;
    }

    std::vector<std::string> getAtonFileList()
    {
        std::vector<std::string> fileList;

        try
        {
            const std::string url = ConfigManager::getInstance().getConfig().api_endpoint.value() + "/v1/atons";
            auto response = getJson(url);
            if (response.body.empty())
                return fileList;

            auto jsonResponse = json::parse(response.body, nullptr, false);
            if (jsonResponse.is_discarded() || !jsonResponse.is_array())
                return fileList;

            for (const auto &item : jsonResponse)
            {
                if (item.is_string())
                {
                    fileList.push_back(item.get<std::string>());
                }
            }
        }
        catch (const std::exception &e)
        {
            // Log error if needed
            std::cerr << "Error fetching file list: " << e.what() << std::endl;
        }

        return fileList;
    }

}
