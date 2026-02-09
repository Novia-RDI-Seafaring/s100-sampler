#pragma once

#include <iostream>
#include <sstream>
#include <pugixml.hpp>
#include <filesystem>
#include <thread>
#include "VTSDigitalInformationMessage.hpp"
#include "DateTime.hpp"
#include <cstring>
#include "S100Parser.hpp"


/**
 * @class VTSParser
 * @brief Parses VTS (Vessel Traffic Service) digital messages from XML.
 *
 * Responsibilities:
 * - Read full VTSDigitalInformationMessage structures
 * - Load subcomponents: ClearanceMessage, VTSLocation
 * - Extract identifiers and reference metadata
 */
class VTSParser : public S100Parser
{
public:
    /**
     * @brief Parses the root VTS message node into a structured object.
     *
     * Extracts UUID, referenceMessageID, and each subfield such as comment,
     * port call ID, voyages, clearance messages, and reporter info.
     *
     * @param doc Fully loaded pugixml document.
     * @return Heap-allocated VTSDigitalInformationMessage instance.
     */
    VTSDigitalInformationMessage *readVTSDigitalInformationMessage(const pugi::xml_document &doc);

    /**
     * @brief Loads a ClearanceMessage block from a nested XML node.
     *
     * Handles field type inference and decoding of numerical and enum fields.
     *
     * @param node XML node named "clearanceMessage".
     * @param newPart Output pointer to populate.
     */
    void loadClearanceMessageFromNode(const pugi::xml_node &node, ClearanceMessage *&newPart);

    /**
     * @brief Parses a VTSLocation node and its attributes.
     *
     * Also handles fallback for unsupported or pending geometry parsing.
     *
     * @param node XML node named "vtsLocation".
     * @param newLoc Output pointer to populate.
     */
    void loadVTSLocationFromNode(const pugi::xml_node &node, VTSLocation *&newLoc);

    /**
     * @brief Extracts the UUID from gml:id attribute of the document root.
     *
     * Removes "id-" prefix if present.
     *
     * @param doc Parsed XML document.
     * @return UUID string.
     */
    std::string readUUID(const pugi::xml_document &doc);

    /**
     * @brief Extracts referenceMessageID from the root element.
     * @param doc Parsed XML document.
     * @return Reference message ID string.
     */
    std::string readRefID(const pugi::xml_document &doc);
};