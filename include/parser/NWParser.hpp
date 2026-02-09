#pragma once

#include "S100Parser.hpp"
#include <unordered_set>

/**
 * @class NWParser
 * @brief Parses NAVWARN XML files into structured NAVWARNPreamble and related entities.
 *
 * Supports:
 * - Reading NAVWARN files and decoding full message structure
 * - Loading parts, references, areas, and metadata fields
 */
class NWParser : public S100Parser
{
public:
    /**
     * @brief Parses an entire NAVWARN XML document into a preamble and components.
     * @param doc Parsed pugixml document.
     * @return Pointer to a fully loaded NAVWARNPreamble.
     */
    NAVWARNPreamble *readMembersFromXML(const pugi::xml_document &doc);

    /**
     * @brief Populates a NAVWARNPart from an XML node.
     * @param node XML node with part structure.
     * @param newPart Output part object.
     */
    void loadPartFromNode(const pugi::xml_node &node, NAVWARNPart *&newPart);

    /**
     * @brief Populates the top-level metadata structure from XML.
     * @param node XML node containing the preamble.
     * @param newPreamble Output preamble object.
     */
    void loadPreambleFromNode(const pugi::xml_node &node, NAVWARNPreamble *&newPreamble);

    /**
     * @brief Parses and loads message reference metadata from XML node.
     * @param node Reference node.
     * @param newReferences Output pointer to be filled.
     */
    void loadReferencesFromNode(const pugi::xml_node &node, References *&newReferences);

    /**
     * @brief Loads geometry and attributes of NAVWARN area blocks.
     * @param node XML node.
     * @param newAreas Output pointer.
     */
    void loadAreasFromNode(const pugi::xml_node &node, NAVWARNAreaAffected *&newAreas);

    /**
     * @brief Parses a warning information node into structured fields.
     * @param warningNode XML node with warning information.
     * @return Parsed WarningInformation structure.
     */
    WarningInformation parseWarningInformation(const pugi::xml_node &warningNode);

    /**
     * @brief Extracts date ranges (start, end) from a node.
     * @param dateNode XML node.
     * @return Populated FixedDateRange.
     */
    FixedDateRange parseFixedDateRange(const pugi::xml_node &dateNode);

    /**
     * @brief Extracts general area attributes like location name and locality.
     * @param generalAreaNode XML node.
     * @return GeneralArea structure.
     */
    GeneralArea parseGeneralAreaNode(const pugi::xml_node &generalAreaNode);

    /**
     * @brief Parses a MessageSeriesIdentifier block (e.g., year, agency).
     * @param messageSeriesIdentifierNode XML node.
     * @return MessageSeriesIdentifier instance.
     */
    MessageSeriesIdentifier parseMessageSeriesIdentifier(const pugi::xml_node &messageSeriesIdentifierNode);
};
