#pragma once

#include <string>
#include <stdexcept>
#include <pugixml.hpp>
#include <map>
#include <vector>
#include <optional>
#include "Logger.hpp"

enum class HttpMethod
{
    HTTP_GET,
    HTTP_POST,
    HTTP_DELETE
};

/*!
 * \struct HttpResponse
 * \brief Encapsulates HTTP response data.
 *
 * \param body Response body as a string.
 * \param statusCode HTTP status code.
 */
struct HttpResponse
{
    std::string body;
    long statusCode;
};

/*!
 * \namespace Curler
 * \brief HTTP utility functions using cURL for REST API operations.
 *
 * Supports GET, POST, and DELETE requests with optional SSL configuration.
 * Includes JSON and Base64 handling, and file utilities.
 */

namespace Curler
{
    /*!
     * \brief Sends an HTTP request to a given URL with optional payload and SSL credentials. Use get, post, etc. as interface for this.
     * \param url Target endpoint.
     * \param payload JSON payload as string (for POST or DELETE).
     * \param method HTTP method (GET, POST, DELETE).
     * \param certPath Optional path to SSL certificate.
     * \param keyPath Optional path to SSL key.
     * \return HttpResponse containing response body and status code.
     * \throws std::runtime_error on cURL failure.
     */
    HttpResponse performRequest(const std::string &url,
                                const std::optional<std::string> &payload,
                                HttpMethod method,
                                const std::optional<std::string> &certPath,
                                const std::optional<std::string> &keyPath);

    /*!
     * \brief Performs a GET request and returns the JSON response.
     */
    HttpResponse getJson(const std::string &url,
                         const std::optional<std::string> &certPath = std::nullopt,
                         const std::optional<std::string> &keyPath = std::nullopt);
    /*!
     * \brief Performs a POST request with a JSON string payload.
     */
    HttpResponse postJson(const std::string &url,
                          const std::string &jsonPayload,
                          const std::optional<std::string> &certPath = std::nullopt,
                          const std::optional<std::string> &keyPath = std::nullopt);
    /*!
     * \brief Performs a DELETE request with a JSON string payload.
     */
    HttpResponse deleteJson(const std::string &url,
                            const std::string &jsonPayload,
                            const std::optional<std::string> &certPath = std::nullopt,
                            const std::optional<std::string> &keyPath = std::nullopt);

    /*!
     * \brief Parses and decodes JSON with base64-encoded XML, then writes to files.
     * \param jsonInput Raw JSON input.
     * \param outputFolder Destination folder for output XML files.
     * \throws std::runtime_error if JSON parsing or file writing fails.
     */
    void saveAndDecodeRawJSONToFiles(const std::string jsonInput, const std::string outputFolder);

    /*!
     * \brief Retrieves a list of available vts names from the server.
     * \return Vector of filenames as strings.
     */
    std::vector<std::string> getFileList();

    /*!
     * \brief Retrieves a list of available aton file names from the server.
     * \return Vector of filenames as strings.
     */
    std::vector<std::string> getAtonFileList();

    /*!
     * \brief Extracts the gml:id attribute from a given XML string.
     * \param xmlData Input XML string.
     * \return Value of gml:id or empty string if not found.
     */
    std::string extractGmlId(const std::string &xmlData);

    /*!
     * \brief Returns a mapping of acknowledgment types from the API.
     * \return Map from string UUID to integer type code.
     */
    std::map<std::string, int> getAllAckTypes();

    static std::size_t callback(
        const char *in,
        std::size_t size,
        std::size_t num,
        std::string *out);
};