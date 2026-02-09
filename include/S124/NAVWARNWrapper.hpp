// NavWarnWrapper.hpp

#pragma once

#include <random>
#include "S100FeatureWrapper.hpp"
#include "NAVWARNPreamble.hpp"

/*!
 * \class NAVWARNWrapper
 * \brief UI-aware wrapper around a parsed NAVWARNPreamble message.
 *
 * Tracks rendering state, prepares `QTreeWidgetItem` for display, and exposes metadata
 * and geometry needed for overlays and interaction. Wraps one NAVWARNPreamble instance.
 */
class NAVWARNWrapper : public S100FeatureWrapper
{
public:
    /*!
     * \brief Constructor. Wraps a NAVWARNPreamble and assigns metadata.
     * \param preamble Pointer to the parsed NAVWARN message.
     */
    NAVWARNWrapper(NAVWARNPreamble *preamble);

    /*!
     * \brief Checks whether the message geometry represents a surface.
     */
    bool isSurface();

    /*!
     * \brief Returns all positions for surface geometry.
     */
    std::vector<DirectPosition *> getSurface();

    /*!
     * \brief Returns the GML ID string from the NAVWARNPreamble.
     */
    std::string getGMLID() const override;

    /*!
     * \brief Returns true if geometry has been parsed.
     */
    bool hasGeometry();

private:
    /*!
     * \brief Generates a basic QTreeWidgetItem from the preamble data.
     *
     * Includes date and type fields.
     */
    QTreeWidgetItem *buildTreeItem() override;
};
