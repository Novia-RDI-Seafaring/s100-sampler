// NAVWARNAreaAffected.cpp

#include "NAVWARNAreaAffected.hpp"

NAVWARNAreaAffected::NAVWARNAreaAffected() {}

void NAVWARNAreaAffected::loadGeometry(Geometry *geo)
{
    geometries.push_back(geo);
}

void NAVWARNAreaAffected::printGeometry()
{
    std::cout << "Printing Geometry information: " << '\n';
    // geometry->print();
}

const std::vector<Geometry *> NAVWARNAreaAffected::getGeometries()
{
    return geometries;
}