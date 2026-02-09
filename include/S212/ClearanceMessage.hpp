#pragma once

#include <string>
#include <optional>
#include <vector>
#include "DateTime.hpp"
#include "subjectOfCommunication.hpp"
#include "messageStatus.hpp"
#include "VTSLocation.hpp"


/*! 
 * \class ClearanceMessage
 * \brief Represents an S-212 clearance message structure.
 *
 * Contains information about the VTS message time window, location and subject. Not sure why this isn't a struct.
 */
class ClearanceMessage
{
public:
    ClearanceMessage();

    SubjectOfCommunication* subjectOfCommunication;
    DateTime effectiveTime;
    MessageStatus* messageStatus;

    std::optional<std::string> windowAfter;
    std::optional<std::string> windowBefore;

    std::optional<double> airDraft;
    std::optional<double> draftOrDraught;

    std::vector<VTSLocation* > vtsLocations;
};
