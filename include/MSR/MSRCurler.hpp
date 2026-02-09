#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "S100Service.hpp"
#include "Curler.hpp"
#include <iostream>
#include <Resources.hpp>

using json = nlohmann::json;

class MSRCurler {
public:
    MSRCurler();

    json getServiceList(const S100Provider &provider, int secomVersion) const;

    json getServiceList(const S100Provider &provider);
    bool pingEndpoint(const QUrl endpoint);

};
