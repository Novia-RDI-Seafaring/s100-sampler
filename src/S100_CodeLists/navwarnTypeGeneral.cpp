#include "NavwarnTypeGeneral.hpp"

// Constructor
NavwarnTypeGeneral::NavwarnTypeGeneral(int v)
{
    warningTypeValue = navwarnTypeGeneralCode(v);
}

NavwarnTypeGeneral::NavwarnTypeGeneral(const std::string &otherTypeName) : otherType(otherTypeName)
{
    warningTypeValue = navwarnTypeGeneralCode(0);
}

std::string NavwarnTypeGeneral::getNavwarnTypeGeneralLabel()
{
    int warningLabelIndex = static_cast<int>(warningTypeValue); // make a check for going over 21
    if (warningLabelIndex == 0)
    {
        return otherType;
    }
    return navwarnTypeGeneralLiterals[warningLabelIndex];
}
const char *const NavwarnTypeGeneral::navwarnTypeGeneralLiterals[21] =
    {
        "Other",
        "Aids to Navigation Changes",
        "Aquaculture and Fishing Installations",
        "Drifting Hazards",
        "ECDIS Operating Anomalies including Official Data Issues",
        "Other Hazards",
        "Health Advisories",
        "Ice Information",
        "In-Force Bulletin",
        "Dangerous Natural Phenomena",
        "Newly Discovered Dangers",
        "Offshore Infrastructure",
        "Piracy or Robbery",
        "Communication or Broadcast Service Change",
        "Routeing Change",
        "Scientific Instruments Change",
        "Security Requirement Change",
        "Special Operations",
        "Towing Operations",
        "Works",
        "Rig List"};
