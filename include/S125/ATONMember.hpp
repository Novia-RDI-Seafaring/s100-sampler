#pragma once

#include "DateTime.hpp"
#include <iostream>
#include <string>
#include "Geometry.hpp"
#include <vector>
#include "S100BaseClass.hpp"
#include <memory>
/*!
 * \struct Status
 * \brief Represents a status record attached to an ATON feature.
 *
 * Intended to be more structured, but currently stores strings directly:
 * - Change type
 * - Description
 * - Optional href
 */
struct Status
{
    std::string changeDetails;
    std::string changeTypes;
    std::string href;
};

/*!
 * \class ATONMember
 * \brief In-memory model of a single parsed S-125 ATON feature.
 *
 * Stores geometry, name, status entries, date, and GML identifier.
 * Used by overlays and wrappers to generate ENC features for rendering.
 */
class ATONMember : public S100BaseDataClass
{
    std::vector<Geometry *> geometries; ///< Owned geometry objects (point/line/polygon)
    std::unique_ptr<Status> status;     ///< Primary status block (takes ownership)
    std::string featureName;            ///< Human-readable feature label
    std::string gmlid;                  ///< GML ID attribute
    std::vector<Status *> statusList;   ///< List of secondary status entries
    Date datasetDate;                   ///< Source dataset publication date

public:
    /*!
     * \brief Constructor. Initializes empty geometry and status containers.
     */
    ATONMember();

    /*!
     * \brief Sets the primary status pointer. Replaces existing.
     * \param newStatus Pointer to a dynamically allocated Status.
     */
    void loadStatus(Status *newStatus);

    /*!
     * \brief Adds an additional status entry to the list.
     */
    void pushStatus(Status *newStatus);

    /*!
     * \brief Adds a geometry object to the member's geometry list.
     */
    void loadGeometry(Geometry *newGeometry);

    /*!
     * \brief Sets the name for this ATON feature.
     */
    void loadName(std::string newName);

    /*!
     * \brief Sets the GML ID from the parsed XML attribute.
     */
    void loadGMLID(std::string newID);

    /*!
     * \brief Sets the dataset publication date.
     */
    void loadDate(Date date);

    /*!
     * \brief Returns the assigned dataset date.
     */
    Date getDate();

    /*!
     * \brief Returns the name of the ATON feature.
     */
    std::string getName();

    /*!
     * \brief Accessor for the primary status block.
     */
    const Status *getStatus() const;

    /*!
     * \brief Returns all additional status entries.
     */
    const std::vector<Status *> getStatusList() const;

    /*!
     * \brief Accessor for the geometry objects (points, lines, etc.).
     */
    const std::vector<Geometry *> &getGeometries() const;

    /*!
     * \brief Returns the GML ID string.
     */
    const std::string &getGMLID() const;

    /*!
     * \brief Returns the first coordinate in the geometry list.
     *
     * Used for placing a single ATON feature on the map.
     */
    DirectPosition *getFirstPosition() const override;
};
