// NAVWARNAreaAffected.hpp

#pragma once

#include <string>
#include <chrono>
#include <vector>

#include "ComplexAttributes.hpp"
#include "Geometry.hpp"

/*!
 * \class NAVWARNAreaAffected
 * \brief Represents the geographical extent of a NAVWARN message.
 *
 * Stores a list of geometries (points, lines, or polygons) indicating
 * the area impacted by a given NAVWARN notice.
 * Serves as a container and access interface for rendering and analysis.
 */
class NAVWARNAreaAffected
{
private:
    std::vector<Geometry *> geometries; ///< List of affected geometries

public:
    /*!
     * \brief Default constructor.
     */
    NAVWARNAreaAffected();

    /*!
     * \brief Adds a geometry object to the affected area list.
     * \param geo Pointer to a Geometry instance.
     */
    void loadGeometry(Geometry *geo);

    /*!
     * \brief Debug utility: prints basic info about contained geometries.
     */
    void printGeometry();

    /*!
     * \brief Returns the list of all affected geometries.
     */
    const std::vector<Geometry *> getGeometries();
};

