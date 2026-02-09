#pragma once

#include <string>
enum class navwarnTypeGeneralCode
{
  Other_Type,
  Aids_to_Navigation_Changes,
  Aquaculture_and_Fishing_Installations,
  Drifting_Hazards,
  ECDIS_Operating_Anomalies_including_Official_Data_Issues,
  Other_Hazards,
  Health_Advisories,
  Ice_Information,
  In_Force_Bulletin,
  Dangerous_Natural_Phenomena,
  Newly_Discovered_Dangers,
  Offshore_Infrastructure,
  Piracy_or_Robbery,
  Communication_or_Broadcast_Service_Change,
  Routeing_Change,
  Scientific_Instruments_Change,
  Security_Requirement_Change,
  Special_Operations,
  Towing_Operations,
  Works,
  Rig_List
};

/*!
 * \class NavwarnTypeGeneral
 * \brief Represents an S-124 warning type
 *
 * Provides conversion from int or string to enum representation,
 * and access to a human-readable string.
 */
class NavwarnTypeGeneral
{
private:
  navwarnTypeGeneralCode warningTypeValue;
  const std::string otherType;
  static const char *const navwarnTypeGeneralLiterals[21];

public:
  /*!
   * \brief Constructor from integer code.
   */
  NavwarnTypeGeneral(int);
  /*!
   * \brief Constructor from string representation.
   */
  NavwarnTypeGeneral(const std::string &);
  /*!
   * \brief Returns the human readable literal
   */
  std::string getNavwarnTypeGeneralLabel();
};