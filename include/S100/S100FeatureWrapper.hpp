#pragma once

#include <string>
#include <QTreeWidgetItem>
#include "S100BaseClass.hpp"

/*!
 * \class S100FeatureWrapper
 * \brief Abstract base class for wrapped S-100 chart features.
 *
 * Handles rendering state and tree item
 */
class S100FeatureWrapper
{
public:
    explicit S100FeatureWrapper(S100BaseDataClass *feature)
        : baseData(feature), drawnState(false), treeItem(nullptr) {}

    virtual ~S100FeatureWrapper()
    {
        delete baseData;
    }

    /*!
     * \brief Returns the unique GML feature ID.
     */
    virtual std::string getGMLID() const = 0;

    /*!
     * \brief Cached UI item for tree display.
     */
    QTreeWidgetItem *getQTreeItem() const
    {
        return treeItem;
    }

    /*!
     * \brief Returns the raw base feature pointer.
     * \note Caller must cast to the expected derived type.
     */
    S100BaseDataClass *getBaseData() const
    {
        return baseData;
    }

    /*!
     * \brief Returns true if rendered.
     */
    bool isDrawn() const
    {
        return drawnState;
    }

    /*!
     * \brief Marks as drawn or not.
     */
    void updateDrawStatus(bool drawStatus)
    {
        drawnState = drawStatus;
    }

protected:
    /*!
     * \brief Derived class must generate the tree item.
     */
    virtual QTreeWidgetItem *buildTreeItem() = 0;

    S100BaseDataClass *baseData;
    QTreeWidgetItem *treeItem;
    bool drawnState;
};
