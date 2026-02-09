#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>
enum class locationTypeCode
{
    From = 1,
    To
};

/*! 
 * \class LocationType
 * \brief Represents an S-212 location type.
 *
 * Provides conversion from int or string to enum representation,
 * and access to a human-readable string.
 */
class LocationType
{
private:
    locationTypeCode locationTypeValue;

    /*!
     * \brief Human-readable string equivalents of locationTypeCode values.
     *
     * Index 0: "From"  
     * Index 1: "To"
     */
    static const char *const locationTypeLiterals[2];

public:
    /*!
     * \brief Constructor from integer code.
     * \param type Integer value (1 for From, 2 for To).
     */
    LocationType(int type);

    /*!
     * \brief Constructor from string representation.
     * \param type String value ("From" or "To").
     */
    LocationType(const std::string &type);
    
    std::string getLocationTypeLiteral();
};