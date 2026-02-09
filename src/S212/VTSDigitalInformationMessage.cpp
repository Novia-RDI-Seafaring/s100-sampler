// VTSDigitalInformationMessage.cpp

#include "VTSDigitalInformationMessage.hpp"

#include <sstream>

VTSDigitalInformationMessage::VTSDigitalInformationMessage() {}

std::string VTSDigitalInformationMessage::toDisplayString()
{
    std::ostringstream oss;

    if (reportedAt)
    {
        oss << "Reported at: " << reportedAt->toDateString() + " " << reportedAt->toTimeString("XML", 3);
    }
    else
    {
        oss << "N/A";
    }
    oss << '\n';

    if (clearancePart[0])
    {

        oss << "Estimated time:" << clearancePart[0]->effectiveTime.toTimeString();
        if (clearancePart[0]->windowAfter)
        {
            std::string lastTwo = clearancePart[0]->windowAfter.value().substr(clearancePart[0]->windowAfter.value().size() - 2);
            oss << " + " << lastTwo << " min";
        }
        oss << '\n';
        if (clearancePart[0]->messageStatus)
            oss << "Status: " << clearancePart[0]->messageStatus->getMessageStatusLiteral() << '\n';
        else
            oss << "Status: N/A\n";

        // if (!clearancePart[0]->vtsLocations.empty() && clearancePart[0]->vtsLocations[0])
        // {
        //     std::string locationURNUnclean = clearancePart[0]->vtsLocations[0]->locationURN.value();
        //     std::string locationCleaned = cutUrn(locationURNUnclean, 2);
        //     oss << "Area: ";
        //     if (clearancePart[0]->vtsLocations[0]->locationType)
        //         oss << clearancePart[0]->vtsLocations[0]->locationType->getLocationTypeLiteral() << " ";
        //     oss << locationCleaned << '\n';
        // }
        // else
        // {
        //     oss << "Area: N/A\n";
        // }
    }
    else
    {
        oss << "Status: N/A\n";
        oss << "Area: N/A\n";
    }

    if (!comment.value().empty())
    {
        oss << "Comment: " << *comment;
    }

    return oss.str();
}

std::vector<std::pair<std::string, std::string>> VTSDigitalInformationMessage::toDisplayFields() {
    std::vector<std::pair<std::string, std::string>> fields;

    QDateTime now = QDateTime::currentDateTime();
    int offset = now.offsetFromUtc() / 3600;
    //std::cout << "offset is" << offset << '\n';
    // Reported at
    if (reportedAt) {
        if ( reportedAt.value().time.m_offset){
            offset = reportedAt.value().time.m_offset;
        }
        fields.emplace_back("Reported at", reportedAt->toDateString() + " " + reportedAt->toTimeString("XML", offset));
    } else {
        fields.emplace_back("Reported at", "N/A");
    }

    // Clearance part
    if (clearancePart[0]) {
        std::string estimated = clearancePart[0]->effectiveTime.toTimeString();

        if (clearancePart[0]->windowAfter) {
            const std::string& wa = clearancePart[0]->windowAfter.value();
            if (wa.size() >= 2) {
                estimated += " + " + wa.substr(wa.size() - 2) + " min";
            }
        }

        fields.emplace_back("Effective time", estimated);

        if (clearancePart[0]->messageStatus) {
            fields.emplace_back("Status", clearancePart[0]->messageStatus->getMessageStatusLiteral());
        } else {
            fields.emplace_back("Status", "N/A");
        }

    } else {
        fields.emplace_back("Status", "N/A");
        fields.emplace_back("Area", "N/A");
    }

    // Comment
    if (!comment.value().empty()) {
        fields.emplace_back("Comment", *comment);
    }

    return fields;
}


std::string VTSDigitalInformationMessage::cutUrn(std::string &urn, int n)
{
    std::vector<std::string> parts;
    std::stringstream ss(urn);
    std::string segment;

    while (std::getline(ss, segment, ':'))
    {
        if (!segment.empty())
        {
            parts.push_back(segment);
        }
    }

    if (n <= 0 || parts.empty())
        return "";

    int startIndex = std::max(0, static_cast<int>(parts.size()) - n);
    std::ostringstream result;
    for (size_t i = startIndex; i < parts.size(); ++i)
    {
        result << parts[i];
        if (i < parts.size() - 1)
        {
            result << " ";
        }
    }

    return result.str();
}
