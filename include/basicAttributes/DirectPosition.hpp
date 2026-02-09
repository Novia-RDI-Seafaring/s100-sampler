#pragma once

#include <iostream>
#include <vector>
#include <string>

/**
 * \class DirectPosition
 * \brief Describes a geographical point.
 */
class DirectPosition
{
public:
    DirectPosition(double latitude, double longitude);

    double getLatitude() const;
    double getLongitude() const;
    std::string printPosition();
private:
    double latitude;
    double longitude;
};