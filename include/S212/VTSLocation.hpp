#pragma once

#include <string>
#include <optional>
#include <vector>
#include "LocationType.hpp"
#include "Geometry.hpp"


/*! 
 * \class VTSLocation
 * \brief Represents an S-212 location data
 *
 * Contains information about the VTS location type, location URN and potential geometry information. 
 *  Not sure why this isn't a struct.
 */
class VTSLocation
{
public:
    VTSLocation();
    LocationType* locationType;

    std::optional<std::string> locationURN;
    std::optional<Geometry *> geometry;

private:
};

