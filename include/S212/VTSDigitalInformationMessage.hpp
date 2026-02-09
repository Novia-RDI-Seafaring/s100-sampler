#pragma once

#include <string>
#include <optional>
#include <vector>
#include "ClearanceMessage.hpp"
#include <QDateTime>


/*! 
 * \class VTSDigitalInformationMessage
 * \brief Represents an S-212 core message.
 *
 * Contains the comment, vesselID, dates, UUID and a list of clearances. 
 * Only one clearance currently supported.
 */
class VTSDigitalInformationMessage
{
public:
    VTSDigitalInformationMessage();

    std::optional<std::string> comment;
    std::optional<std::string> incomingVoyage;
    std::optional<std::string> journeyID;
    std::optional<std::string> outgoingVoyage;
    std::optional<std::string> portCallID;
    std::optional<std::string> referenceMessageID;
    std::optional<DateTime> reportedAt;
    std::optional<std::string> reportedBy;
    std::optional<std::string> vesselID;
    std::string uuid;
    std::vector<ClearanceMessage*> clearancePart;

    /*!
     * \brief  DEPRACATED. Returns a preprocessed string.
     */
    std::string toDisplayString();
    
     /*!
     * \brief Returns a list of pairs of fields for later visual processing.
     */
    std::vector<std::pair<std::string, std::string>> toDisplayFields();

private:
     /*!
     * \brief Picks n-elements from an URN.
     */
    std::string cutUrn(std::string& urn, int n);
};