// NAVWARNPart.hpp

#pragma once

#include <string>
#include <chrono>
#include <vector>

#include "ComplexAttributes.hpp"
#include "Geometry.hpp"
#include "S100BaseClass.hpp"

/*!
 * \class NAVWARNPart
 * \brief Represents a structured part of a NAVWARN message.
 *
 * Encapsulates:
 * - Feature names and references
 * - Fixed date ranges
 * - Associated geometries
 * - Warning information block
 *
 * Used as a building block within a NAVWARNPreamble.
 */
class NAVWARNPart : S100BaseDataClass
{
private:
    std::vector<FeatureName> featureNames; ///< List of associated feature names
    std::vector<FeatureReference> featureReferences; ///< Linked feature references
    std::vector<FixedDateRange> fixedDateRanges; ///< Time period(s) for message validity
    WarningInformation warningInformation; ///< Description and context of the warning
    std::vector<Geometry *> geometries; ///< Geometrical areas affected by the warning

public:
    /*!
     * \brief Default constructor.
     */
    NAVWARNPart();

    /*!
     * \brief Adds a fixed date range to the part.
     */
    void loadFDR(FixedDateRange &);

    /*!
     * \brief Loads the main warning information block.
     */
    void loadWI(WarningInformation &);

    /*!
     * \brief Adds a geometry object to the list.
     */
    void loadGeometry(Geometry *);

    /*!
     * \brief Prints the part's geometry info to stdout (debug only).
     */
    void printGeometry();

    /*!
     * \brief Prints part-level debug info including warning text.
     */
    void printPart();

    /*!
     * \brief Returns all geometries associated with this part.
     */
    const std::vector<Geometry *> getGeometries();

    /*!
     * \brief Returns all feature name entries.
     */
    const std::vector<FeatureName> getFeatureNames();

    /*!
     * \brief Returns all feature references.
     */
    const std::vector<FeatureReference> getFeatureReferences();

    /*!
     * \brief Returns all fixed date ranges linked to this part.
     */
    const std::vector<FixedDateRange> getfixedDateRanges();

    /*!
     * \brief Returns the main warning information block.
     */
    const WarningInformation getWarningInformation();

    /*!
     * \brief Returns the first coordinate of the first geometry.
     *
     * Used for positioning symbols on overlays. Assumes at least one geometry exists.
     */
    DirectPosition* getFirstPosition() const override;
};

