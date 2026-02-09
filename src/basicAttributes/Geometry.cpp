#include "Geometry.hpp"

// Constructor for GM_Point
GM_Point::GM_Point(double lat, double lon)
{
    position = new DirectPosition(lat, lon);
}

GM_Point::GM_Point(DirectPosition *pos)
{
    position = pos;
}

GM_Surface::GM_Surface(std::vector<DirectPosition *> posList)
{
    positions = posList;
}

const std::vector<DirectPosition *> GM_Surface::getPositions() const
{
    return positions;
}

const std::vector<DirectPosition *> GM_Point::getPositions() const
{
    std::vector<DirectPosition *> positions = {position};
    return positions;
}

std::string GM_Point::getGeometryType() const {
    return "Point";
}

std::string GM_Surface::getGeometryType() const {
    return "Surface";
}