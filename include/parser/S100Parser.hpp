#pragma once

#include <iostream>
#include <sstream>
#include "ComplexAttributes.hpp"
#include "NAVWARNPreamble.hpp"
#include "NAVWARNAreaAffected.hpp"
#include "References.hpp"
#include "Geometry.hpp"
#include <pugixml.hpp>
#include <filesystem>
#include <thread>

/**
 * @class S100Parser
 * @brief Abstract base class providing shared XML parsing utilities for S-100 derived datasets.
 *
 * Not intended to be used directly. Provides:
 * - Namespace-stripping logic
 * - Recursive XML node search
 * - Coordinate parsing into DirectPosition
 * - Geometry interpretation (points, curves, surfaces)
 */
class S100Parser
{
public:
    /**
     * @brief Default constructor.
     */
    explicit S100Parser() = default;

    /**
     * @brief Virtual destructor.
     */
    virtual ~S100Parser() = default;

protected:


    /**
     * @brief Returns the first file found in a directory, without waiting.
     * @param folderPath Directory to scan.
     * @return File path or empty string.
     */
    std::string findNewFile(const std::string &folderPath);

    /**
     * @brief Recursively searches an XML node tree for a node with the given name.
     *
     * Strips namespaces before comparison.
     *
     * @param node Root XML node to search.
     * @param nodeName Target node name (no namespace).
     * @return Matching pugi::xml_node or empty node if not found.
     */
    pugi::xml_node findNodeRec(const pugi::xml_node &node, std::string nodeName);

    /**
     * @brief Removes XML namespace prefix from a node's tag name.
     * @param node XML node whose tag name may include a namespace.
     * @return Cleaned tag name string.
     */
    std::string clearNameSpace(const pugi::xml_node &node);

    /**
     * @brief Parses a string of space-separated lat/lon pairs into DirectPosition pointers.
     *
     * Expected format: "lat lon lat lon ..."
     *
     * @param latLongString Raw coordinate string.
     * @return Vector of heap-allocated DirectPosition pointers.
     */
    std::vector<DirectPosition *> parseCoordinates(const std::string &latLongString);

    /**
     * @brief Interprets a geometry node into a GM_Point or GM_Surface based on contents.
     *
     * Recognizes:
     * - gml:pointProperty → GM_Point
     * - gml:surfaceProperty / curveProperty → GM_Surface
     *
     * @param geometryNode XML node wrapping the geometry.
     * @return Pointer to parsed Geometry object or nullptr if not recognized.
     */
    Geometry *parseGeometry(const pugi::xml_node &geometryNode);
};

