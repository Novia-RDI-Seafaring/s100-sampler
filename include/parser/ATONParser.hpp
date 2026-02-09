#pragma once

#include "S100Parser.hpp"
#include <unordered_set>
#include "ATONMember.hpp"
#include <unordered_map>

/**
 * @class ATONParser
 * @brief Parses Aids to Navigation (ATON) XML data into structured ATONMember objects.
 *
 * Responsibilities:
 * - Extract AtoNStatusInformation entries
 * - Match and attach StatusPart references to ATON members
 * - Parse geometries and names for display and interaction
 */
class ATONParser : public S100Parser
{
public:
    /**
     * @brief Parses all ATON members from the given XML document.
     *
     * Filters out entries that lack a StatusPart.
     * Links referenced status blocks using xlink:href.
     *
     * @param doc Parsed pugixml document.
     * @return Vector of parsed ATONMember pointers.
     */
    std::vector<ATONMember *> readMembersFromXML(const pugi::xml_document &doc);

    /**
     * @brief Parses AtoNStatusInformation nodes and builds a lookup table.
     *
     * Expects gml:id to be used for matching from StatusPart hrefs.
     *
     * @param doc XML document.
     * @param statusMap Output map: ID -> Status*
     */
    void parseStatuses(const pugi::xml_document &doc, std::unordered_map<std::string, Status *> &statusMap);
};
