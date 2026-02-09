#pragma once

/**
 * \file ComplexAttributes.hpp
 * \brief Contains data structures used in NAVWARN messages
 *
 * Includes types for chart information, feature references, location names,
 * fixed date ranges, and message identifiers.
 * Used across S-100 data processing. All types are not used.
 * Some are not even perfectly according to the standard
 */

#include <string>
#include <chrono>
#include <iostream>
#include <optional>

#include "TruncatedDate.hpp"
#include "Time.hpp"
#include "NavwarnTypeGeneral.hpp"
#include "navwarnTypeDetails.hpp"
#include "WarningType.hpp"


/**
 * \struct ChartAffected
 * \brief Describes a chart affected by a NAVWARN, including identifiers and edition dates.
 */
struct ChartAffected
{
    std::string chartNumber;
    std::string chartPlanNumber;
    Date editionDate;
    Date lastNoticeDate;
};

/**
 * \struct AffectedChartPublications
 * \brief Contains publication-specific information about affected charts.
 */
struct AffectedChartPublications
{
    ChartAffected chartAffected;
    std::string chartPublicationIdentifier;
    std::string internationalChartAffected;
    std::string language; ///< ISO 639-2 three-letter language code
    std::string publicationAffected;
};

/**
 * \struct LocationName
 * \brief Localized name of a location.
 */
struct LocationName
{
    std::string language;
    std::string text;
};

/**
 * \struct NAVWARNtitle
 * \brief Language-tagged title of a NAVWARN message.
 */
struct NAVWARNtitle
{
    std::string language;
    std::string text;
};

/**
 * \struct GeneralArea
 * \brief General area affected by the message, optionally includes a locality identifier.
 */
struct GeneralArea
{
    std::optional<std::string> localityIdentifier;
    LocationName location;
};

/**
 * \struct Locality
 * \brief A defined locality including an identifier and name.
 */
struct Locality
{
    std::string localityIdentifier;
    LocationName location;
};

/**
 * \struct FeatureName
 * \brief Represents a named feature with language and display preference.
 */
struct FeatureName
{
    bool displayName;
    std::string language;
    std::string name;
};

/**
 * \struct ENCFeatureReference
 * \brief Identifies an ENC feature instance with edition and update metadata.
 */
struct ENCFeatureReference
{
    std::string editionNumber;
    std::string ENCName;
    std::string featureObjectIdentifier; ///< One or more identifiers
    std::string updateNumber;
};

/**
 * \struct FeatureReference
 * \brief References a feature affected by the message, including AtoN and ENC metadata.
 */
struct FeatureReference
{
    std::string AtoNNumber;
    std::string featureIdentifier;
    ENCFeatureReference ENCFeatureRef;
};

/**
 * \struct FixedDateRange
 * \brief Describes a time interval using start and end dates and times.
 */
struct FixedDateRange
{
    TruncatedDate dateEnd;
    TruncatedDate dateStart;
    Time TimeOfDayEnd;
    Time TimeOfDayStart;
};

/**
 * \struct Information
 * \brief Free-text information with a language tag.
 */
struct Information
{
    std::string language;
    std::string text;
};

/**
 * \struct MessageSeriesIdentifier
 * \brief Describes the identification details of a message series.
 */
struct MessageSeriesIdentifier
{
    std::string agencyResponsibleForProduction;
    std::optional<std::string> countryName;
    std::string nameOfSeries;
    std::optional<std::string> warningIdentifier;
    int warningNumber;
    S100_WarningType *warningType;
    int year;

    /**
     * \brief Prints the struct contents for debugging.
     */
    void printStruct()
    {
        printf("Agency Responsible for Production: %s \n", agencyResponsibleForProduction.c_str());
        // printf("Country Name: %s \n", countryName.c_str());
        printf("Name of Series: %s \n", nameOfSeries.c_str());
        printf("Warning Number: %d \n", warningNumber);
        printf("Warning Type: %s \n", warningType->getWarningTypeLabel().c_str());
        printf("Year: %d \n", year);
    }
};

/**
 * \struct WarningInformation
 * \brief Container for free-text warning and its corresponding details type.
 */
struct WarningInformation
{
    Information information;
    NavwarnTypeDetails* navwarnTypeDetails;
};
