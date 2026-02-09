#include "DirectPosition.hpp"

// Constructor
DirectPosition::DirectPosition(double lat, double lon) : latitude(lat), longitude(lon) {}

// Getter for latitude
double DirectPosition::getLatitude() const
{
    return latitude;
}

// Getter for longitude
double DirectPosition::getLongitude() const
{
    return longitude;
}

std::string DirectPosition::printPosition()
{
    std::string coordinates;
    coordinates += "(" + std::to_string(latitude) + ", " + std::to_string(longitude) + ")";
    return coordinates;
}