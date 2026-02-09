#ifndef SIMPARSER_HPP
#define SIMPARSER_HPP

#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <nlohmann/json.hpp>
#include "DateTime.hpp"

//Consider making optional fields
struct AISData
{
    std::string measurement;
    DateTime start;
    DateTime stop;
    DateTime time;
    double cog;
    double heading;
    double lat;
    double lon;
    double sog;
    std::string mmsi;
    int navStat;
    bool posAcc;
    std::string result;
    int rot;
    int table;
    std::string type;
};
/**
 * @brief Not used
 */
namespace SimParser
{

    using json = nlohmann::json;

    // Reads the file and returns a vector of strings representing JSON objects
    std::optional<std::vector<std::string>> readFile(const std::string &);

    // Filters entries by the given MMSI and returns a vector of JSON objects
    std::optional<std::vector<json>> filterByMMSI(const std::vector<std::string> &, const std::string &);
    std::string preprocessLogEntry(const std::string &entry);
    AISData from_json(const nlohmann::json &j);

    std::vector<std::unique_ptr<AISData>> nonJSONFilesetToAISDataset(const std::string & filename, const std::string & mmsi);

}

#endif // SIMPARSER_HPP
