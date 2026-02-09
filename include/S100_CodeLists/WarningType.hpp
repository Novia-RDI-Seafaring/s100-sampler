#pragma once

#include <string>

enum class WarningTypeCode
{
    Local_Navigational_Warning,
    Coastal_Navigational_Warning,
    Sub_Area_Navigational_Warning,
    NAVAREA_Navigational_Warning,
    NAVAREA_No_Warning,
    Sub_Area_No_Warning,
    Coastal_No_Warning,
    Local_No_Warning,
    NAVAREA_In_Force_Bulletin,
    Sub_Area_In_Force_Bulletin,
    Coastal_In_Force_Bulletin,
    Local_In_Force_Bulletin
};

/*! 
 * \class S100_WarningType
 * \brief Represents an S-124 warning type
 *
 * Provides conversion from int or string to enum representation,
 * and access to a human-readable string.
 */
class S100_WarningType
{
private:
    WarningTypeCode warningTypeValue;
    static const char *const warningTypeLabelLiterals[12];

public:
    /*!
     * \brief Constructor from integer code.
     */
    S100_WarningType(int v);
         /*!
     * \brief Returns the human readable literal
     */
    std::string getWarningTypeLabel();
};