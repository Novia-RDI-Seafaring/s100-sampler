#include "VTSParser.hpp"

std::string VTSParser::readUUID(const pugi::xml_document &doc)
{
    pugi::xml_node root = doc.first_child();

    // Extract the gml:id attribute and remove the "id-" prefix
    const char *gmlId = root.attribute("gml:id").value();
    if (gmlId && std::strncmp(gmlId, "id-", 3) == 0)
    {
        return std::string(gmlId + 3); // Skip the "id-" part
    }
    else
    {
        return std::string(gmlId);
    }
}

std::string VTSParser::readRefID(const pugi::xml_document &doc)
{
    pugi::xml_node root = doc.first_child();
    // Extract the referenceMessageID attribute
    const char *refMessageID = root.attribute("referenceMessageID").value();
    //std::cout << "RefMessage ID" << refMessageID << '\n';
    return std::string(refMessageID);
}

VTSDigitalInformationMessage *VTSParser::readVTSDigitalInformationMessage(const pugi::xml_document &doc)
{

    VTSDigitalInformationMessage *newMessage = new VTSDigitalInformationMessage();

    pugi::xml_node root = doc.first_child();
    auto vtsDetails = root.children();

    // Extract the gml:id attribute and remove the "id-" prefix
    const char *gmlId = root.attribute("gml:id").value();
    if (gmlId && std::strncmp(gmlId, "id-", 3) == 0)
    {
        newMessage->uuid = std::string(gmlId + 3); // Skip the "id-" part
    }
    else
    {
        newMessage->uuid = std::string(gmlId);
    }

    // Extract the referenceMessageID attribute
    const char *refMessageID = root.attribute("referenceMessageID").value();
    if (refMessageID)
    {
        newMessage->referenceMessageID = std::string(refMessageID);
    }

    for (const pugi::xpath_node &vtsDetailPath : vtsDetails)
    {
        // Get the underlying node from the xpath_node
        pugi::xml_node detailNode = vtsDetailPath.node();
        std::string detailName = clearNameSpace(detailNode);
        if (detailName == "comment")
        {
            newMessage->comment = detailNode.text().get();
        }
        // if (detailName == "referenceMessageID")
        // {
        //     newMessage->referenceMessageID = detailNode.text().get();
        // }
        else if (detailName == "outgoingVoyage")
        {
            newMessage->outgoingVoyage = detailNode.text().get();
        }
        else if (detailName == "incomingVoyage")
        {
            newMessage->incomingVoyage = detailNode.text().get();
        }
        else if (detailName == "portCallID")
        {
            newMessage->portCallID = detailNode.text().get();
        }
        else if (detailName == "reportedAt")
        {
            newMessage->reportedAt = DateTime(detailNode.text().get());
        }
        else if (detailName == "reportedBy")
        {
            newMessage->reportedBy = detailNode.text().get();
        }
        else if (detailName == "vesselID")
        {
            newMessage->vesselID = detailNode.text().get();
        }
        else if (detailName == "clearanceMessage")
        {
            ClearanceMessage *clearanceMessage = new ClearanceMessage();

            loadClearanceMessageFromNode(detailNode, clearanceMessage);
            newMessage->clearancePart.push_back(clearanceMessage);
        }
    }
    return newMessage;
}

void VTSParser::loadClearanceMessageFromNode(const pugi::xml_node &node, ClearanceMessage *&newClearance)
{
    for (const pugi::xml_node &detailNode : node.children())
    {
        std::string detailName = clearNameSpace(detailNode);

        if (detailName == "airDraft")
        {
            newClearance->airDraft = std::stod(detailNode.text().get());
        }
        else if (detailName == "draftOrDraught")
        {
            newClearance->draftOrDraught = std::stod(detailNode.text().get());
        }
        else if (detailName == "effectiveTime")
        {
            newClearance->effectiveTime = DateTime(detailNode.text().get());
        }
        else if (detailName == "messageStatus")
        {
            if (detailNode.attribute("code"))
            {
                int statusCode = std::stoi(detailNode.attribute("code").value());
                newClearance->messageStatus = new MessageStatus(statusCode);
            }
            else
            {
                // If no 'code', use the text value directly
                newClearance->messageStatus = new MessageStatus(detailNode.text().get()); // Assuming MessageStatus can handle string literals
            }
        }
        else if (detailName == "subjectOfCommunication")
        {
            // Check if 'code' attribute exists
            if (detailNode.attribute("code"))
            {
                int statusCode = std::stoi(detailNode.attribute("code").value());
                newClearance->subjectOfCommunication = new SubjectOfCommunication(statusCode);
            }
            else
            {
                // If no 'code', use the text value directly
                newClearance->subjectOfCommunication = new SubjectOfCommunication(detailNode.text().get());
            }
        }
        else if (detailName == "windowAfter")
        {
            newClearance->windowAfter = detailNode.text().get();
        }
        else if (detailName == "windowBefore")
        {
            newClearance->windowBefore = detailNode.text().get();
        }
        else if (detailName == "vtsLocation")
        {
            VTSLocation *vtsLocation = new VTSLocation();
            loadVTSLocationFromNode(detailNode, vtsLocation);
            newClearance->vtsLocations.push_back(vtsLocation);
        }
    }
}

void VTSParser::loadVTSLocationFromNode(const pugi::xml_node &node, VTSLocation *&vtsLocation)
{

    for (const pugi::xml_node &detailNode : node.children())
    {
        std::string detailName = clearNameSpace(detailNode);

        if (detailName == "locationType")
        {
            if (detailNode.attribute("code"))
            {
                int statusCode = std::stoi(detailNode.attribute("code").value());
                vtsLocation->locationType = new LocationType(statusCode);
            }
            else
            {
                vtsLocation->locationType = new LocationType(detailNode.text().get());
            }
        }
        else if (detailName == "locationURN")
        {
            vtsLocation->locationURN = detailNode.text().get();
        }
        else if (detailName == "arcByCenterPoint" ||
                 detailName == "pointProperty" ||
                 detailName == "polygonProperty" ||
                 detailName == "circleByCenterPoint")
        {
            std::cout << "General geometry not yet implemented";
            // TODO
            // vtsLocation->geometry = parseGeometry(detailNode); // Assume parseGeometry returns a Geometry* object
        }
    }
}