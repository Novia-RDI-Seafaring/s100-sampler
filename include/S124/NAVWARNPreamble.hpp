// NAVWARNPreamble.hpp

#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <optional>

#include "NAVWARNPart.hpp"
#include "NAVWARNAreaAffected.hpp"
#include "References.hpp"
#include "DateTime.hpp"
#include "NavwarnTypeGeneral.hpp"
#include "navwarnTypeDetails.hpp"

/*!
 * \class NAVWARNPreamble
 * \brief Represents the top-level container for a NAVWARN message.
 *
 * Encapsulates:
 * - Message series metadata (type, location, ID)
 * - List of NAVWARNParts with detailed warnings
 * - Area affected and reference metadata
 * - Publication and cancellation dates
 */
class NAVWARNPreamble : public S100BaseDataClass
{
public:
    /*!
     * \brief Default constructor.
     */
    NAVWARNPreamble();

    /*!
     * \brief Loads the message series identifier (e.g., type, country, number).
     */
    void loadMSI(MessageSeriesIdentifier);

    /*!
     * \brief Adds a general area object to the message.
     */
    void loadGA(GeneralArea);

    /*!
     * \brief Sets whether the warning is currently in force.
     */
    void loadIntService(bool);

    /*!
     * \brief Sets the cancellation date, if applicable.
     */
    void loadCancellationDate(const std::optional<DateTime> &);

    /*!
     * \brief Sets the publication timestamp.
     */
    void loadPublicationTime(const std::optional<DateTime> &);

    /*!
     * \brief Loads the general NAVWARN type from an integer code.
     */
    void loadNAVWARNType(int);

    /*!
     * \brief Loads the general NAVWARN type from a descriptive string.
     */
    void loadNAVWARNType(const std::string &);

    /*!
     * \brief Loads the GML ID from the XML source.
     */
    void loadGMLID(std::string);

    /*!
     * \brief Returns the GML ID string.
     */
    const std::string &getGMLID() const;

    /*!
     * \brief Appends a new NAVWARNPart pointer to the content list.
     */
    void appendNewPart(NAVWARNPart *&);

    /*!
     * \brief Appends a new reference entry.
     */
    void appendNewReference(References *&);

    /*!
     * \brief Appends a new area-affected object.
     */
    void appendNewAreaAffected(NAVWARNAreaAffected *&);

    /*!
     * \brief Prints the preamble and all parts to stdout (debug).
     */
    void printPreamble();

    /*!
     * \brief Debug method to print geometry of first part.
     */
    void printGeometry();

    /*!
     * \brief Generates a formatted string describing the full NAVWARN message.
     *
     * Includes general info and each part's warning information and geometry.
     */
    std::string getPreambleData();

    /*!
     * \brief Returns the first NAVWARNPart (used for simplified access).
     */
    NAVWARNPart *returnPart();

    /*!
     * \brief Returns chart publication metadata (optional).
     */
    const std::optional<std::vector<AffectedChartPublications>> &getAffectedChartPublications() const;

    /*!
     * \brief Returns general area(s) referenced in the preamble.
     */
    const std::vector<GeneralArea> &getGeneralAreas() const;

    /*!
     * \brief Returns optional locality information.
     */
    const std::optional<std::vector<Locality>> &getLocalities() const;

    /*!
     * \brief Returns the message series metadata.
     */
    const MessageSeriesIdentifier &getMessageSeriesIdentifier() const;

    /*!
     * \brief Returns title metadata for the message, if any.
     */
    const std::optional<std::vector<NAVWARNtitle>> &getNavwarnTitles() const;

    /*!
     * \brief Returns the cancellation date, if available.
     */
    const std::optional<DateTime> &getCancellationDate() const;

    /*!
     * \brief Returns whether the NAVWARN is currently in service.
     */
    bool getIntService() const;

    /*!
     * \brief Returns the parsed NAVWARN type info.
     */
    NavwarnTypeGeneral *getNavwarnTypeGeneral() const;

    /*!
     * \brief Returns the publication time, if available.
     */
    const std::optional<DateTime> &getPublicationTime() const;

    /*!
     * \brief Returns all NAVWARN parts (content blocks).
     */
    const std::vector<NAVWARNPart *> &getNAVWARNPreambleContent() const;

    /*!
     * \brief Returns reference metadata entries.
     */
    const std::vector<References *> &getNAVWARNReferences() const;

    /*!
     * \brief Returns first geometry coordinate in the message.
     */
    DirectPosition *getFirstPosition() const override;

private:
    std::optional<std::vector<AffectedChartPublications>> affectedChartPublications;
    std::vector<GeneralArea> generalAreas;
    std::optional<std::vector<Locality>> localities;
    MessageSeriesIdentifier messageSeriesIdentifier;
    std::optional<std::vector<NAVWARNtitle>> navwarnTitles;

    std::optional<DateTime> cancellationDate;
    bool intService;
    NavwarnTypeGeneral *navwarnTypeGeneral;
    std::optional<DateTime> publicationTime;

    std::vector<NAVWARNPart *> NAVWARNPreambleContent;
    std::vector<References *> NAVWARNReferences;
    std::vector<NAVWARNAreaAffected *> NAVWARNAreas;
    std::string gmlid;
};