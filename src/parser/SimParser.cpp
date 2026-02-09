#include "SimParser.hpp"
#include <iostream>
#include <sstream> // For std::istringstream and std::ostringstream

namespace SimParser
{

    AISData from_json(const nlohmann::json &j)
    {
        AISData data;

        // Extracting data from the JSON object and assigning to the struct

        DateTime startTime = DateTime(j["_start"]);
        DateTime stopTime = DateTime(j["_stop"]);
        DateTime time = DateTime(j["_time"]);

        data.measurement = j["_measurement"];
        data.start = startTime;
        data.stop = stopTime;
        data.time = time;
        data.cog = j["cog"];
        data.heading = j["heading"];
        data.lat = j["lat"];
        data.lon = j["lon"];
        data.mmsi = j["mmsi"];
        data.navStat = j["navStat"];
        data.posAcc = j["posAcc"];
        data.result = j["result"];
        data.rot = j["rot"];
        data.table = j["table"];
        data.type = j["type"];

        return data;
    }

    std::optional<std::vector<std::string>> readFile(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            return std::nullopt;
        }

        std::vector<std::string> entries;
        std::string line, entry;
        while (std::getline(file, line))
        {
            entry += line; // Append each line
            if (!line.empty() && line.back() == '}')
            { // End of a JSON object
                entries.push_back(entry);
                entry.clear();
            }
        }

        if (!entry.empty())
        {
            std::cerr << "Unfinished JSON entry: " << entry << "\n"; // Log unfinished entries
        }

        return entries;
    }

    std::optional<std::vector<json>> filterByMMSI(const std::vector<std::string> &entries, const std::string &targetMMSI)
    {
        std::vector<json> filteredEntries;

        for (const auto &entry : entries)
        {
            try
            {
                std::string processedEntry = preprocessLogEntry(entry); // Preprocess the entry
                json j = json::parse(processedEntry);                   // Parse the JSON
                // std::cout << "Parsed entry: " << j.dump() << "\n";      // Debug output

                // Check if the MMSI field exists and matches the target
                if (j.contains("mmsi"))
                {
                    if ((j["mmsi"].is_string() && j["mmsi"] == targetMMSI) ||
                        (j["mmsi"].is_number_integer() && std::to_string(j["mmsi"].get<int>()) == targetMMSI))
                    {
                        filteredEntries.push_back(j);
                    }
                }
            }
            catch (const json::parse_error &e)
            {
                std::cerr << "JSON parse error: " << e.what() << "\n";
                std::cerr << "Problematic entry: " << entry << "\n";
            }
            catch (const std::exception &e)
            {
                std::cerr << "Unexpected error: " << e.what() << "\n";
            }
        }

        if (filteredEntries.empty())
        {
            return std::nullopt; // Return empty optional if no matches are found
        }
        return filteredEntries;
    }

    std::string preprocessLogEntry(const std::string &input)
    {
        std::string result = input;

        // Replace single quotes with double quotes for JSON compliance
        std::replace(result.begin(), result.end(), '\'', '"');

        // Replace Python-style True/False with JSON-style true/false
        size_t pos = 0;
        while ((pos = result.find(" True", pos)) != std::string::npos)
        {
            result.replace(pos, 5, "true");
        }
        pos = 0;
        while ((pos = result.find(" False", pos)) != std::string::npos)
        {
            result.replace(pos, 6, "false");
        }

        // Process datetime strings to remove Python `datetime.datetime` formatting
        pos = 0;
        while ((pos = result.find("datetime.datetime(", pos)) != std::string::npos)
        {
            size_t start = pos + std::string("datetime.datetime(").length();
            size_t end = result.find(")", start);

            // Extract the datetime parts and format
            std::string datetime = result.substr(start, end - start);
            std::istringstream stream(datetime);
            std::ostringstream formatted;
            std::string year, month, day, hour, minute, second;

            std::getline(stream, year, ',');
            std::getline(stream, month, ',');
            std::getline(stream, day, ',');
            std::getline(stream, hour, ',');
            std::getline(stream, minute, ',');
            std::getline(stream, second, ',');

            // Remove any extra spaces that may exist in the components
            formatted << year.substr(year.find_first_not_of(" ")) << "-"
                      << std::setw(2) << std::setfill('0') << month.substr(month.find_first_not_of(" ")) << "-"
                      << std::setw(2) << std::setfill('0') << day.substr(day.find_first_not_of(" ")) << "T"
                      << std::setw(2) << std::setfill('0') << hour.substr(hour.find_first_not_of(" ")) << ":"
                      << std::setw(2) << std::setfill('0') << minute.substr(minute.find_first_not_of(" ")) << ":"
                      << std::setw(2) << std::setfill('0') << second.substr(0, second.find('.')).substr(second.find_first_not_of(" ")) << "Z";

            // Replace the original datetime with the formatted version
            result.replace(pos, end - pos + 1, "\"" + formatted.str() + "\"");
            pos += formatted.str().length();
        }

        return result;
    }
    std::vector<std::unique_ptr<AISData>> nonJSONFilesetToAISDataset(const std::string &filename, const std::string &mmsi)
    {
        std::vector<std::unique_ptr<AISData>> aisDataSet;
        auto entriesOpt = SimParser::readFile(filename);
        if (!entriesOpt)
        {
            std::cerr << "Failed to open file: " << filename << "\n";
            return aisDataSet;
        }
        for (const auto &entry : entriesOpt.value())
        {
            std::cout << entry << '\n';
            break;
        }
        auto filteredOpt = SimParser::filterByMMSI(*entriesOpt, mmsi);
        if (!filteredOpt)
        {
            std::cout << "No entries found for MMSI: " << mmsi << "\n";
            return aisDataSet;
        }
        for (const auto j : filteredOpt.value())
        {
            std::unique_ptr<AISData> newAIS = std::make_unique<AISData>();

            DateTime startTime = DateTime(j["_start"]);
            DateTime stopTime = DateTime(j["_stop"]);
            DateTime time = DateTime(j["_time"]);

            newAIS->measurement = j["_measurement"];
            newAIS->start = startTime;
            newAIS->stop = stopTime;
            newAIS->time = time;
            newAIS->cog = j["cog"];
            newAIS->heading = j["heading"];
            newAIS->lat = j["lat"];
            newAIS->lon = j["lon"];
            newAIS->mmsi = j["mmsi"];
            newAIS->navStat = j["navStat"];
            newAIS->posAcc = j["posAcc"];
            newAIS->result = j["result"];
            newAIS->rot = j["rot"];
            newAIS->table = j["table"];
            newAIS->type = j["type"];

            aisDataSet.push_back(std::move(newAIS));
        }
        return aisDataSet;
    }

}
