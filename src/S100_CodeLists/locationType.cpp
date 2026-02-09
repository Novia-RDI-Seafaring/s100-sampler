#include "LocationType.hpp"

LocationType::LocationType(int v)
{
    locationTypeValue = locationTypeCode(v);
}

LocationType::LocationType(const std::string &subject)
{
    // Find the subject string in the literals list
    auto it = std::find(std::begin(locationTypeLiterals), std::end(locationTypeLiterals), subject);

    // If found, assign the corresponding enum value
    if (it != std::end(locationTypeLiterals))
    {
        locationTypeValue = locationTypeCode(std::distance(std::begin(locationTypeLiterals), it));
    }
    else
    {
        // Throw an exception if the subject string is invalid
        throw std::invalid_argument("Invalid subject of communication string: " + subject);
    }
}

std::string LocationType::getLocationTypeLiteral()
{
    int labelIndex = static_cast<int>(locationTypeValue);

    return locationTypeLiterals[labelIndex];
}
const char *const LocationType::locationTypeLiterals[2] =
    {
        "From",
        "To"};
