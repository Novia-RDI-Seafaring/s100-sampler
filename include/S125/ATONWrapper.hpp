#pragma once

#include <QTreeWidgetItem>
#include "ATONMember.hpp"
#include <string>
#include <vector>
#include "S100FeatureWrapper.hpp"

/*!
 * \class ATONWrapper
 * \brief Wraps a single ATONMember with UI metadata and draw state.
 *
 * Provides accessors for geometry, status, and attributes, and builds a
 * QTreeWidgetItem for display in the application. Tracks whether the ATON
 * has been drawn to the chart layer.
 */
class ATONWrapper : public S100FeatureWrapper
{
public:
    /*!
     * \brief Constructor. Wraps an ATONMember and generates a tree item.
     * \param member Pointer to the parsed ATONMember object.
     */
    ATONWrapper(ATONMember *);

    /*!
     * \brief Accessor for the member's parsed Status block.
     */
    const Status *getStatus();

    /*!
     * \brief Returns geometries associated with the ATON feature.
     */
    std::vector<Geometry *> getGeometries();

    /*!
     * \brief Returns the GML ID string associated with this ATON.
     */
    std::string getGMLID() const override;

private:
    Status *atonStatus; ///< Status block from ATONMember

    /*!
     * \brief Generates a basic QTreeWidgetItem from the member data.
     *
     * Includes date, type, and change description fields.
     */
    QTreeWidgetItem *buildTreeItem() override;
};
