#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <toml++/toml.h>
#include <optional>


struct Config
{
  std::optional<std::string> api_endpoint;

  std::optional<std::string> cert;
  std::optional<std::string> key;

  int app_id;
  int customer_id;
  std::optional<std::map<std::string, std::vector<std::string>>> location_map;
  std::optional<std::string> vessel_id;
  std::optional<std::string> all_approaches;
  std::optional<std::string> turku_harbor;
  std::optional<std::string> general_charts;
  std::optional<std::string> danish_charts;

  std::optional<std::string> com_port;
  std::optional<std::string> vtg_handle;
  std::optional<std::string> gga_handle;
  std::optional<std::string> hdt_handle;
  std::optional<std::string> gll_handle;
  int baud_rate;
};

class ConfigManager
{
public:
  // Access the single instance of the configuration
  static ConfigManager &getInstance()
  {
    static ConfigManager instance; // Singleton instance
    return instance;
  }

  // Delete copy constructor and assignment operator
  ConfigManager(const ConfigManager &) = delete;
  ConfigManager &operator=(const ConfigManager &) = delete;

  // Method to load configuration from a TOML file
  void LoadConfig(const std::string &filename = "../config.toml")
  {
    try
    {
      auto config = toml::parse_file(filename);

      // Load values into the configuration object
      cfg_.api_endpoint = config["network"]["api_endpoint"].value<std::string>();

      cfg_.cert = config["network"]["certpath"].value<std::string>();
      cfg_.key = config["network"]["keypath"].value<std::string>();

      cfg_.vessel_id = config["ship"]["vessel_id"].value<std::string>();

      cfg_.app_id = config["nautilus"]["app_id"].value_or(0);
      cfg_.customer_id = config["nautilus"]["customer_id"].value_or(0);

      cfg_.all_approaches = config["charts"]["allApproaches"].value<std::string>();
      cfg_.turku_harbor = config["charts"]["turkuHarbor"].value<std::string>();
      cfg_.general_charts = config["charts"]["generalCharts"].value<std::string>();
      cfg_.danish_charts = config["charts"]["danishCharts"].value<std::string>();

      cfg_.com_port = config["nmea"]["comport"].value<std::string>();
      cfg_.gga_handle = config["nmea"]["GGA"].value<std::string>();
      cfg_.hdt_handle = config["nmea"]["HDT"].value<std::string>();
      cfg_.gll_handle = config["nmea"]["GLL"].value<std::string>();
      cfg_.vtg_handle = config["nmea"]["VTG"].value<std::string>();
      cfg_.baud_rate = config["nmea"]["baud"].value_or(0);

      // Load locations into the map
      if (auto locations = config["locations"].as_table())
      {
        std::map<std::string, std::vector<std::string>> location_map;
        for (const auto &[key, value] : *locations)
        {
          if (auto array = value.as_array())
          {
            std::vector<std::string> loc_list;
            for (const auto &item : *array)
            {
              if (auto loc = item.value<std::string>())
              {
                loc_list.push_back(*loc);
              }
            }
            location_map[std::string(key)] = std::move(loc_list);
          }
        }
        cfg_.location_map = std::move(location_map);
      }
    }
    catch (const toml::parse_error &err)
    {
      std::cerr << "Parsing failed: " << err.description() << "\n";
    }
  }

  // Getter for configuration
  const Config &getConfig() const
  {
    return cfg_;
  }

private:
  ConfigManager() = default; // Private constructor to prevent instantiation

  Config cfg_; // The actual configuration data
};

namespace Resources
{

  const inline char *readSVGFile(const std::string &filename)
  {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file)
    {
      std::cerr << "Error opening file: " << filename << std::endl;
      return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char *buffer = new char[size + 1];
    if (!file.read(buffer, size))
    {
      std::cerr << "Error reading file: " << filename << std::endl;
      delete[] buffer;
      return nullptr;
    }
    buffer[size] = '\0'; // Null-terminate
    return buffer;
  }

};
