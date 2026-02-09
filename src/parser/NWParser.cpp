// NNParser.cpp

#include "NWParser.hpp"
#include "DateTime.hpp"

MessageSeriesIdentifier NWParser::parseMessageSeriesIdentifier(const pugi::xml_node &messageSeriesIdentifierNode)
{
    MessageSeriesIdentifier dataStruct;

    for (const pugi::xml_node &field : messageSeriesIdentifierNode.children())
    {
        std::string fieldName = clearNameSpace(field);
        auto fieldValue = field.text().get();
        auto fieldCode = field.attribute("code").value();

        if (fieldName == "agencyResponsibleForProduction")
        {
            dataStruct.agencyResponsibleForProduction = fieldValue;
        }
        else if (fieldName == "countryName")
        {
            dataStruct.countryName = fieldValue;
        }
        else if (fieldName == "nameOfSeries")
        {
            dataStruct.nameOfSeries = fieldValue;
        }
        else if (fieldName == "warningNumber")
        {
            //std::cout << "warning number " << fieldValue << '\n';
            try
            {
                dataStruct.warningNumber = std::stoi(fieldValue);
            }
            catch (const std::invalid_argument &e)
            {
                // Handle the case where fieldValue cannot be converted to an integer
                std::cerr << "Invalid value for warningNumber: " << e.what() << std::endl;
            }
            //std::cout << "warning number passed" << '\n';
        }
        else if (fieldName == "warningType")
        {
            //std::cout << "warning number " << fieldValue << '\n';

            try
            {
                dataStruct.warningType = new S100_WarningType(std::stoi(fieldCode));
            }
            catch (const std::invalid_argument &e)
            {
                // Handle the case where fieldValue cannot be converted to an integer
                std::cerr << "Invalid value for warningType: " << e.what() << std::endl;
            }
            //std::cout << "warning type passed" << '\n';
        }
        else if (fieldName == "year")
        {
            try
            {
                dataStruct.year = std::stoi(fieldValue);
            }
            catch (const std::invalid_argument &e)
            {
                // Handle the case where fieldValue cannot be converted to an integer
                std::cerr << "Invalid value for year: " << e.what() << std::endl;
            }
        }
        else
        {
            // Handle unknown field names
        }
    }
    return dataStruct;
}

GeneralArea NWParser::parseGeneralAreaNode(const pugi::xml_node &generalAreaNode)
{
    GeneralArea dataStruct;

    for (const pugi::xml_node &field : generalAreaNode.children())
    {
        auto fieldValue = field.text().get();
        std::string fieldName = clearNameSpace(field);

        if (fieldName == "localityIdentifier")
        {
            dataStruct.localityIdentifier = fieldValue;
        }
        else if (fieldName == "locationName")
        {
            LocationName newLocation;
            auto textPosition = findNodeRec(field, "text");
            newLocation.text = textPosition.text().get();

            auto languagePosition = findNodeRec(field, "language");
            newLocation.language = languagePosition.text().get();

            dataStruct.location = newLocation;
        }
        else
        {
            // Handle unknown field names
        }
    }
    return dataStruct;
}

FixedDateRange NWParser::parseFixedDateRange(const pugi::xml_node &dateNode)
{
    FixedDateRange dataStruct;

    for (const pugi::xml_node &field : dateNode.children())
    {
        std::string fieldName = clearNameSpace(field);
        auto fieldValue = field.text().get();

        if (fieldName == "dateStart")
        {

            std::string dateInfo;
            for (auto childField : field.children())
            {
                std::string childFieldName = clearNameSpace(childField);
                if (childFieldName == "date")
                {
                    dateInfo = childField.text().get();
                }
            }
            TruncatedDate *newDate = new TruncatedDate(dateInfo);
            dataStruct.dateStart = *newDate;
        }
        if (fieldName == "dateEnd")
        {

            std::string dateInfo;
            for (auto childField : field.children())
            {
                std::string childFieldName = clearNameSpace(childField);
                if (childFieldName == "date")
                {
                    dateInfo = childField.text().get();
                }
            }
            TruncatedDate *newDate = new TruncatedDate(dateInfo);
            dataStruct.dateEnd = *newDate;
        }
    }
    return dataStruct;
}

WarningInformation NWParser::parseWarningInformation(const pugi::xml_node &warningNode)
{
    WarningInformation dataStruct;

    for (const pugi::xml_node &field : warningNode.children())
    {
        std::string fieldName = clearNameSpace(field);
        auto fieldValue = field.text().get();

        if (fieldName == "information")
        {
            Information newInformation;

            for (auto childField : field.children())
            {
                std::string childFieldName = clearNameSpace(childField);
                if (childFieldName == "language")
                {
                    newInformation.language = childField.text().get();
                }
                if (childFieldName == "text")
                {
                    newInformation.text = childField.text().get();
                }
            }
            dataStruct.information = newInformation;
            continue;
        }
        if (fieldName == "navwarnTypeDetails")
        {
            for (auto childField : field.children())
            {
                std::string childFieldName = clearNameSpace(childField);
                if (childFieldName == "navwarnTypeDetails")
                {
                    std::string navwarnTypeDetailsCode = field.attribute("code").value();
                    //std::cout << "Navwarn type details code: " << navwarnTypeDetailsCode << '\n';
                    if (navwarnTypeDetailsCode == "other")
                    {
                        auto navwarnTypeDetailsString = childField.text().get();
                        auto *newNavwarnTypeDetail = new NavwarnTypeDetails(navwarnTypeDetailsString);
                        dataStruct.navwarnTypeDetails = newNavwarnTypeDetail;
                    }
                    else
                    {
                        auto *newNavwarnTypeDetail = new NavwarnTypeDetails(std::stoi(navwarnTypeDetailsCode));
                        dataStruct.navwarnTypeDetails = newNavwarnTypeDetail;
                    }
                    //std::cout << "passed" << '\n';
                }
            }
            continue;
        }
    }
    return dataStruct;
}

void NWParser::loadPreambleFromNode(const pugi::xml_node &node, NAVWARNPreamble *&newPreamble)
{
    for (const pugi::xpath_node &navwarn_part : node)
    {
        pugi::xml_node field = navwarn_part.node();
        auto fieldValue = field.text().get();
        std::string fieldName = clearNameSpace(field);
        //std::cout << "Loading MSI" << '\n';
        if (fieldName == "messageSeriesIdentifier")
        {
            MessageSeriesIdentifier MSI = parseMessageSeriesIdentifier(field);
            newPreamble->loadMSI(MSI);
            continue;
        }
        //std::cout << "Loading Ga" << '\n';

        if (fieldName == "generalArea")
        {
            GeneralArea GA = parseGeneralAreaNode(field);
            newPreamble->loadGA(GA);
            continue;
        }
        if (*fieldValue == '\0') // Order defines what fields are optional for now
        {
            // printf("Field: %s is empty '\n'", fieldName.c_str());
            continue;
        };
        //std::cout << "Loading PT" << '\n';

        if (fieldName == "publicationTime")
        {
            std::string dateTimeString = field.text().get();
            if (dateTimeString.empty())
            {
                continue;
            }

            size_t tPos = dateTimeString.find("-");

            if (tPos == std::string::npos)
            {
                std::cerr << "Publication time probably wrong format" << '\n';
                continue;
            }
            //std::cout << "PT is " << dateTimeString << '\n';
            DateTime newDateTime = DateTime(dateTimeString);
            newPreamble->loadPublicationTime(newDateTime);
            continue;
        }
        //std::cout << "Loading CT" << '\n';

        if (fieldName == "cancellationDate")
        {
            std::string dateTimeString = field.text().get();

            size_t tPos = dateTimeString.find("-");

            if (tPos == std::string::npos)
            {
                std::cerr << "cancellationDate probably wrong format" << '\n';
                continue;
            }

            DateTime newDateTime = DateTime(dateTimeString);
            newPreamble->loadCancellationDate(newDateTime);
            continue;
        }
        //std::cout << "Loading IS" << '\n';

        if (fieldName == "intService")
        {
            bool intservice = field.text().as_bool();
            newPreamble->loadIntService(intservice);
            continue;
        }
        //std::cout << "Loading NTG" << '\n';

        if (fieldName == "navwarnTypeGeneral")
        {
            std::string navwarnTypeCode = field.attribute("code").value();
            if (navwarnTypeCode == "other" || navwarnTypeCode.empty())
            {
                const std::string navwarntypeText = field.text().get();
                newPreamble->loadNAVWARNType(navwarntypeText);
                continue;
            }
            // Load Standard NavwarnType based on the int
            //std::cout << "Navwarn type  code: " << navwarnTypeCode << '\n';

            newPreamble->loadNAVWARNType(std::stoi(navwarnTypeCode));
            //std::cout << "passed" << '\n';
            continue;
        }
    }
}

void NWParser::loadPartFromNode(const pugi::xml_node &node, NAVWARNPart *&newPart)
{
    for (const pugi::xpath_node &navwarn_part : node)
    {
        // Get the underlying node from the xpath_node
        pugi::xml_node field = navwarn_part.node();
        auto fieldValue = field.text().get();
        std::string fieldName = clearNameSpace(field);

        if (fieldName == "fixedDateRange")
        {
            std::string dateTimeString = field.text().get();
            size_t tPos = dateTimeString.find("-");

            if (tPos == std::string::npos)
            {
                std::cerr << "fixedDateRange probably wrong format" << '\n';
                continue;
            }

            FixedDateRange FDR = parseFixedDateRange(field);
            newPart->loadFDR(FDR);
            continue;
        }

        if (fieldName == "warningInformation")
        {

            WarningInformation WI = parseWarningInformation(field);
            newPart->loadWI(WI);
            continue;
        }
        if (fieldName == "geometry")
        {
            Geometry *geo = parseGeometry(field);
            newPart->loadGeometry(geo);
            continue;
        }
        if (*fieldValue == '\0')
        {
            // printf("Field: %s is empty '\n'", fieldName.c_str());
            continue;
        };
    }
}

void NWParser::loadReferencesFromNode(const pugi::xml_node &node, References *&newReferences)
{
    for (const pugi::xpath_node &reference : node)
    {
        // Get the underlying node from the xpath_node
        pugi::xml_node field = reference.node();
        auto fieldValue = field.text().get();
        std::string fieldName = clearNameSpace(field);
        if (fieldName == "messageSeriesIdentifier")
        {
            MessageSeriesIdentifier MSI = parseMessageSeriesIdentifier(field);
            newReferences->loadMSI(MSI);
            continue;
        }

        if (fieldName == "noMessageOnHand")
        {
            bool messageOnHand = field.text().as_bool();
            newReferences->setNoMessageOnHand(messageOnHand);
            continue;
        }
        if (fieldName == "referenceCategory")
        {
            auto referenceCategory = std::stoi(field.attribute("code").value());
            newReferences->setReferenceCategory(referenceCategory);
            continue;
        }
        if (*fieldValue == '\0')
        {
            // printf("Field: %s is empty '\n'", fieldName.c_str());
            continue;
        };
    }
}

void NWParser::loadAreasFromNode(const pugi::xml_node &node, NAVWARNAreaAffected *&newAreas)
{
    for (const pugi::xpath_node &reference : node)
    {
        // Get the underlying node from the xpath_node
        pugi::xml_node field = reference.node();
        auto fieldValue = field.text().get();
        std::string fieldName = clearNameSpace(field);
        if (fieldName == "geometry")
        {
            Geometry *geo = parseGeometry(field);
            newAreas->loadGeometry(geo);
            continue;
        }
        if (*fieldValue == '\0')
        {
            continue;
        };
    }
}

NAVWARNPreamble *NWParser::readMembersFromXML(const pugi::xml_document &doc)
{

    auto navwarnMembers = doc.children(); // Temporary
    NAVWARNPreamble *newPreamble = new NAVWARNPreamble();
    pugi::xml_node root = doc.first_child();
    //std::cout << " Reading members " << '\n';
    for (pugi::xml_node child : root.children())
    {
        auto nodeName = clearNameSpace(child);
        if (nodeName == "members")
        {
            navwarnMembers = child.children();
        }
    }
    pugi::xml_attribute gmlIDAttr = root.attribute("gml:id");

    if (gmlIDAttr)
    {
        std::string gmlID = gmlIDAttr.value();
        newPreamble->loadGMLID(gmlID);
    }
    else
    {
        //<ns4:NavwarnPreamble ns1:id="DK.NW-199-25">
        pugi::xml_node preambleNode = doc.select_node("//*[local-name()='NavwarnPreamble']").node();
        if (preambleNode)
        {
            pugi::xml_attribute idAttr = preambleNode.attribute("ns1:id");
            newPreamble->loadGMLID(idAttr.value());
        }
    }
    //std::cout << " GML found " << newPreamble->getGMLID() << '\n';

    for (const pugi::xpath_node &navwarnMemberPath : navwarnMembers)
    {
        // Get the underlying node from the xpath_node
        pugi::xml_node navwarnMemberNode = navwarnMemberPath.node();
        std::string memberName = clearNameSpace(navwarnMemberNode);

        //std::cout << "Loading preamble " << '\n';

        if (memberName == "NAVWARNPreamble" || memberName == "NavwarnPreamble")
        {
            loadPreambleFromNode(navwarnMemberNode, newPreamble);
            continue;
        }
        //std::cout << "Loading part " << '\n';

        if (memberName == "NAVWARNPart" || memberName == "NavwarnPart")
        {
            NAVWARNPart *newPart = new NAVWARNPart();

            loadPartFromNode(navwarnMemberNode, newPart);
            newPreamble->appendNewPart(newPart);

            continue;
        }
        //std::cout << "Loading refs " << '\n';

        if (memberName == "References")
        {
            References *newReference = new References();

            loadReferencesFromNode(navwarnMemberNode, newReference);
            newPreamble->appendNewReference(newReference);
            continue;
        }
        //std::cout << "Loading area " << '\n';

        if (memberName == "NAVWARNAreaAffected")
        {
            NAVWARNAreaAffected *newArea = new NAVWARNAreaAffected();

            loadAreasFromNode(navwarnMemberNode, newArea);
            newPreamble->appendNewAreaAffected(newArea);
            continue;
        }
    }
    return newPreamble;
}
