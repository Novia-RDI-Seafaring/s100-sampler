#include "ATONParser.hpp"
#include "DateTime.hpp"

// Parses ATONMembers from XML, **only keeps members with Statuspart**
std::vector<ATONMember *> ATONParser::readMembersFromXML(const pugi::xml_document &doc)
{
    std::vector<ATONMember *> memberList;
    std::unordered_map<std::string, Status *> statusMap; // Store AtoNStatusInformation
    parseStatuses(doc, statusMap);                       // First, extract all status information
    // std::cout << "Status amount found: " << statusMap.size() << '\n';

    pugi::xml_node root = doc.first_child();
    pugi::xml_node membersNode = root.child("members"); // Check for "members" node
    pugi::xml_node metadata = root.child("S100:DatasetIdentificationInformation");
    // std::cout << metadata.child("S100:datasetReferenceDate").text().get() << '\n';
    Date datasetDate = Date(metadata.child("S100:datasetReferenceDate").text().get());
    pugi::xml_node parentNode = membersNode ? membersNode : root; // Determine processing node

    for (pugi::xml_node member : parentNode.children())
    {
        auto nodeName = clearNameSpace(member);
        pugi::xml_node child = member.first_child();
        if (!child)
            continue;

        bool hasStatusPart = false;
        std::string href;
        std::vector<std::string> hrefList;

        // **First Pass: Look for Statuspart**
        for (pugi::xml_node field : member.children())
        {
            std::string fieldName = clearNameSpace(field);
            if (fieldName == "Statuspart")
            {
                hasStatusPart = true;
                href = field.attribute("xlink:href").as_string();

                // Check if href contains a hash mark
                if (href.find('#') != std::string::npos)
                {
                    href = href.substr(href.find('#') + 1);
                }

                hrefList.push_back(href);
                // std::cout << "Found Statuspart with href: " << href << '\n';
                //  break; // For now only one status
            }
        }

        if (!hasStatusPart)
        {
            // std::cout << "Skipping member, no Statuspart found.\n";
            continue; // Skip this member if no Statuspart exists
        }

        // **Second Pass: Process the Member since it has a Statuspart**
        ATONMember *newMember = new ATONMember();
        pugi::xml_attribute gmlIDAttr = member.attribute("gml:id");

        if (gmlIDAttr)
        {
            std::string gmlID = gmlIDAttr.value();
            newMember->loadGMLID(gmlID);
        }

        if (!href.empty() && statusMap.find(href) != statusMap.end())
        {
            newMember->loadStatus(statusMap[href]); // Link Statuspart to AtoNStatusInformation
            for (auto statusref : hrefList)
            {
                newMember->pushStatus(statusMap[statusref]);
            }
        }

        // Process other fields
        for (pugi::xml_node field : member.children())
        {
            std::string fieldName = clearNameSpace(field);
            // std::cout << "Processing field: " << fieldName << '\n';

            if (fieldName == "geometry")
            {
                Geometry *geo = parseGeometry(field);
                newMember->loadGeometry(geo);
            }
            else if (fieldName == "featureName")
            {
                newMember->loadName(field.child("name").text().get());
            }
        }

        newMember->loadDate(datasetDate);

        memberList.push_back(newMember);
    }
    return memberList;
}

// Parses the Statuspart nodes and links them with AtoNStatusInformation
void ATONParser::parseStatuses(const pugi::xml_document &doc, std::unordered_map<std::string, Status *> &statusMap)
{
    // Find all AtoNStatusInformation nodes and store them in a map
    pugi::xpath_node_set statusNodes = doc.select_nodes("//AtoNStatusInformation");

    for (const pugi::xpath_node &xpathNode : statusNodes)
    {
        // std::cout << "Looping statuses " << '\n';
        pugi::xml_node statusInfo = xpathNode.node();
        std::string id = statusInfo.attribute("gml:id").as_string();
        // std::string changeDetails = statusInfo.child("changeDetails").child("fixedAtonChange").text().as_string();
        std::string changeDetails = "";
        for (const auto changeChild : statusInfo.child("changeDetails").children())
        {
            changeDetails = changeChild.text().as_string();
        }
        std::string changeType = statusInfo.child("ChangeTypes").text().as_string();
        // std::cout << "id " << id << '\n';

        if (!id.empty())
        {
            Status *newStatus = new Status();
            newStatus->changeDetails = changeDetails;
            newStatus->changeTypes = changeType;
            newStatus->href = id;
            statusMap[id] = newStatus;
        }
    }
}
