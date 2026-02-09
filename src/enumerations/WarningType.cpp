#include "WarningType.hpp"

// Constructor
S100_WarningType::S100_WarningType(int v)
{
    warningTypeValue = WarningTypeCode(v - 1); //navwarn codes are offset by one
}
std::string S100_WarningType::getWarningTypeLabel()
{
    int warningLabelIndex = static_cast<int>(warningTypeValue); // make a check for going over 12
    return warningTypeLabelLiterals[warningLabelIndex];
}

const char *const S100_WarningType::warningTypeLabelLiterals[12] =
    {
        "Local Navigational Warning",
        "Coastal Navigational Warning",
        "Sub-Area Navigational Warning",
        "NAVAREA Navigational Warning",
        "NAVAREA No Warning",
        "Sub-Area No Warning",
        "Coastal No Warning",
        "Local No Warning",
        "NAVAREA In-Force Bulletin",
        "Sub-Area In-Force Bulletin",
        "Coastal In-Force Bulletin",
        "Local In-Force Bulletin"};