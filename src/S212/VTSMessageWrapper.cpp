// VTSMessageWrapper.cpp

#include "VTSMessageWrapper.hpp"
#include <qdebug.h>
#include <QSvgRenderer>
#include <QPainter>
#include <QProcess>
#include <filesystem>
#include <stdexcept>

VTSMessageWrapper::VTSMessageWrapper(VTSDigitalInformationMessage *newMessage, Sender vtsOrShip)
{
    sender = vtsOrShip;
    message = newMessage;
    try
    {
        listItem = generateQListWidgetItem();
        decorateItem();
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception caught:" << e.what();
    }
}

VTSMessageWrapper::~VTSMessageWrapper()
{
    delete message;
}

void VTSMessageWrapper::generateUUID()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

    std::stringstream ss;

    ss << std::hex << std::setfill('0');

    // Generate 8-4-4-4-12 format
    ss << std::setw(8) << dis(gen) << '-';                       // 8 characters
    ss << std::setw(4) << (dis(gen) & 0xFFFF) << '-';            // 4 characters
    ss << std::setw(4) << ((dis(gen) & 0x0FFF) | 0x4000) << '-'; // 4 characters, version 4
    ss << std::setw(4) << ((dis(gen) & 0x3FFF) | 0x8000) << '-'; // 4 characters, variant
    ss << std::setw(4) << (dis(gen) & 0xFFFF)                    // First half of 12 characters
       << std::setw(8) << dis(gen);                              // Second half of 12 characters

    uuid = ss.str();
    filename = uuid + ".xml";
}

QListWidgetItem *VTSMessageWrapper::generateQListWidgetItem()
{
    QListWidgetItem *rootItem = new QListWidgetItem();
    rootItem->setSizeHint(QSize(0, 175)); // Size hint still belongs here
    rootItem->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(message)));
    return rootItem;
}

void VTSMessageWrapper::decorateItem()
{
    // Determine sender properties
    QString senderText;
    switch (sender)
    {
    case Sender::VTS:
        senderText = "VTS";
        break;
    case Sender::Ship:
        senderText = "SHIP";
        break;
    default:
        senderText = "Unknown";
        break;
    }

    vtsMessage = new VTSMessageWidget(senderText, message->toDisplayFields());
}


QListWidgetItem *VTSMessageWrapper::getQListWidgetItem()
{
    return listItem;
}

void VTSMessageWrapper::writeToFile()
{
    std::cout << "Writing to file" << '\n';

    pugi::xml_document doc;

    // Add root element with namespaces and attributes
    auto root = doc.append_child("vtsDigitalInformationMessage");
    root.append_attribute("xmlns:gml") = "http://www.opengis.net/gml/3.2";
    root.append_attribute("xmlns") = "http://www.iho.int/S212/gml/cs0/1.0";
    root.append_attribute("xmlns:S100") = "http://www.iho.int/s100gml/5.0";
    root.append_attribute("xmlns:xsi") = "http://www.w3.org/2001/XMLSchema-instance";
    root.append_attribute("gml:id") = "id-"; //"id-133c3a8e-c073-470a-92d9-b7b1ac337d3b";
    root.append_attribute("xsi:schemaLocation") = "http://www.iho.int/S212/gml/cs0/1.0 S212.xsd";

    pugi::xml_attribute attr = root.attribute("gml:id");
    std::cout << "Pugi roots ok" << '\n';

    // Append the UUID to the existing value
    std::string new_value = std::string(attr.value()) + uuid;
    attr.set_value(new_value.c_str());

    // Add optional fields if present
    if (message->comment)
    {
        root.append_child("comment").text() = message->comment->c_str();
    }
    if (message->outgoingVoyage)
    {
        root.append_child("outgoingVoyage").text() = "urn:mrn:foobar:voyage:example_voyage"; // outgoingVoyage->c_str();
    }
    if (message->incomingVoyage)
    {
        root.append_child("incomingVoyage").text() = "urn:mrn:foobar:voyage:example_voyage"; // incomingVoyage->c_str();
    }
    if (message->referenceMessageID)
    {
        root.append_attribute("referenceMessageID") = message->referenceMessageID->c_str();
    }
    if (message->portCallID)
    {
        root.append_child("portCallID").text() = message->portCallID->c_str();
    }
    if (message->reportedAt)
    {
        root.append_child("reportedAt").text() = message->reportedAt->toString().c_str();
    }
    if (message->reportedBy)
    {
        root.append_child("reportedBy").text() = message->reportedBy->c_str(); // reportedBy->c_str();
    }
    if (message->vesselID)
    {
        root.append_child("vesselID").text() = message->vesselID->c_str();
    }
    std::cout << "Main fields ok" << '\n';

    // Add clearanceMessage elements
    std::cout << "Starting clearance" << '\n';

    for (const auto &clearance : message->clearancePart)
    {
        auto clearanceNode = root.append_child("clearanceMessage");
        clearanceNode.append_attribute("gml:id") = "clearancegmlid-24242"; // clearance->gmlId->c_str();

        auto boundedBy = clearanceNode.append_child("gml:boundedBy");
        auto envelope = boundedBy.append_child("gml:Envelope");
        envelope.append_attribute("srsName") = "urn:ogc:def:crs:EPSG:4326";
        envelope.append_attribute("srsDimension") = "2";
        envelope.append_child("gml:lowerCorner").text() = "3.1";
        envelope.append_child("gml:upperCorner").text() = "3.14";

        if (clearance->airDraft)
        {
            clearanceNode.append_child("airDraft").text() = std::to_string(clearance->airDraft.value()).c_str();
        }
        if (clearance->draftOrDraught)
        {
            clearanceNode.append_child("draftOrDraught").text() = std::to_string(clearance->draftOrDraught.value()).c_str();
        }

        clearanceNode.append_child("effectiveTime").text() = clearance->effectiveTime.toString().c_str();

        if (clearance->messageStatus)
        {
            clearanceNode.append_child("messageStatus").text() = clearance->messageStatus->getMessageStatusLiteral().c_str();
        }
        if (clearance->subjectOfCommunication)
        {
            clearanceNode.append_child("subjectOfCommunication").text() = clearance->subjectOfCommunication->getSubjectOfCommunicationLiteral().c_str();
        }
        if (clearance->windowAfter)
        {
            clearanceNode.append_child("windowAfter").text() = clearance->windowAfter->c_str();
        }
        if (clearance->windowBefore)
        {
            clearanceNode.append_child("windowBefore").text() = clearance->windowBefore->c_str();
        }

        for (const auto &vtsLocation : clearance->vtsLocations)
        {
            auto vtsLocNode = clearanceNode.append_child("vtsLocation");
            vtsLocNode.append_attribute("gml:id") = "vtsgmlid-2222"; // vtsLocation->gmlId->c_str();

            if (vtsLocation->locationType)
            {
                vtsLocNode.append_child("locationType").text() = vtsLocation->locationType->getLocationTypeLiteral().c_str();
            }
            if (vtsLocation->locationURN)
            {
                vtsLocNode.append_child("locationURN").text() = vtsLocation->locationURN->c_str();
            }
        }
    }
    std::cout << " clearance ok" << '\n';

    // Save XML to file
    std::string fullPath = "../data/VTS/" + filename;
    qDebug() << "Saving file to:" << QString::fromStdString(fullPath);
    doc.save_file(fullPath.c_str(), "  ");
    if (!validateXML(fullPath))
    {
        // Do not keep or send a message that failed schema validation
        std::filesystem::remove(fullPath);
        throw std::runtime_error("S-212 schema validation failed for " + filename +
                                 ". Check that ../schemas/S212.xsd exists and xmllint is available.");
    }
}
bool VTSMessageWrapper::validateXML(const std::string &filePath)
{
#ifdef XMLLINT_EXECUTABLE
    const QString xmllint = QStringLiteral(XMLLINT_EXECUTABLE);
#else
    const QString xmllint = QStringLiteral("xmllint");
#endif
    // Pass arguments as a list so that no shell is involved
    const QStringList args{"--noout", "--schema", "../schemas/S212.xsd", QString::fromStdString(filePath)};
    int result = QProcess::execute(xmllint, args);

    if (result == 0)
    {
        std::cout << "XML file is valid.\n";
        return true;
    }
    if (result < 0)
        std::cerr << "XML validation failed: could not run " << xmllint.toStdString() << "\n";
    else
        std::cerr << "XML validation failed!\n";
    return false;
}

std::string VTSMessageWrapper::getReferenceId()
{
    if (!message->referenceMessageID.has_value())
    {
        std::cout << "Reference ID is empty or uninitialized" << '\n';
        return ""; // Return an empty string for missing IDs
    }
    return message->referenceMessageID.value();
}

std::string VTSMessageWrapper::getPortCallID()
{
    if (!message->portCallID.has_value())
    {
        std::cout << "Reference ID is empty or uninitialized" << '\n';
        return ""; // Return an empty string for missing IDs
    }
    return message->portCallID.value();
}

DateTime VTSMessageWrapper::getReportTime()
{
    if (!message->reportedAt.has_value())
    {
        std::cout << "ReportTime missing" << '\n';
        return DateTime(0, 0, 0, 0, 0, 0);
    }
    return message->reportedAt.value();
}

std::string VTSMessageWrapper::getSubject()
{
    // message->toDisplayString();
    return message->clearancePart[0]->subjectOfCommunication->getSubjectOfCommunicationLiteral();
}

std::string VTSMessageWrapper::getLocation()
{
    auto stringer = message->clearancePart[0]->vtsLocations[0]->locationURN.value();
    return stringer;
}

DateTime VTSMessageWrapper::getEffectiveTime()
{
    return message->clearancePart[0]->effectiveTime;
}
std::string VTSMessageWrapper::getWindowAfter()
{
    if (message->clearancePart[0]->windowAfter)
    {
        return message->clearancePart[0]->windowAfter.value();
    }
    return "00:00";
}