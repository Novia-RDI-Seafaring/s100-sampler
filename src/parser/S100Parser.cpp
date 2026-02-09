#include "S100Parser.hpp"

std::string S100Parser::findNewFile(const std::string &folderPath)
{
    namespace fs = std::filesystem;

    fs::path folder(folderPath);
    std::cout << folder << '\n';
    // Check if the folder exists
    if (!fs::exists(folder) || !fs::is_directory(folder))
    {
        std::cerr << "Folder does not exist or is not a directory.\n";
        return "";
    }

    for (const auto &entry : fs::directory_iterator(folder))
    {
        std::cout << entry << '\n';

        // Check if it's a regular file and not processed before
        if (fs::is_regular_file(entry.path()))
        {
            return entry.path().string();
        }
    }
    return "";
}

pugi::xml_node S100Parser::findNodeRec(const pugi::xml_node &node, std::string nodeName)
{
    for (const pugi::xml_node &subNode : node.children())
    {
        std::string subNodeName = clearNameSpace(subNode);

        if (subNodeName == nodeName)
        {
            return subNode;
        }

        // Recursively search in child nodes
        pugi::xml_node foundNode = findNodeRec(subNode, nodeName);
        if (foundNode)
        {
            return foundNode; // Return if found in child nodes
        }
    }

    // Return empty node if not found
    return pugi::xml_node();
}

std::string S100Parser::clearNameSpace(const pugi::xml_node &field)
{
    auto fieldName = std::string(field.name());
    auto pos = fieldName.find(':'); // Find the position of the colon

    if (pos != std::string::npos)
    {
        fieldName = fieldName.substr(pos + 1); // Extract the part after the colon
    }
    return fieldName;
}

std::vector<DirectPosition *> S100Parser::parseCoordinates(const std::string &latLongString)
{
    std::istringstream iss(latLongString);
    std::vector<DirectPosition *> coordinates;
    double latitude, longitude;

    while (iss >> latitude >> longitude)
    {
        DirectPosition *newDirPosition = new DirectPosition(latitude, longitude);
        coordinates.push_back(newDirPosition);
    }

    return coordinates;
}

Geometry *S100Parser::parseGeometry(const pugi::xml_node &geometryNode)
{
    for (const pugi::xml_node &field : geometryNode.children())
    {
        std::string fieldName = clearNameSpace(field);
        auto fieldValue = field.text().get();

        if (fieldName == "pointProperty")
        {
            auto position = findNodeRec(field, "pos");

            if (position == pugi::xml_node())
            {
                return nullptr;
            }

            std::string latLongString = position.text().get();
            std::size_t spacePos = latLongString.find(' ');
            std::string latitude = latLongString.substr(0, spacePos);
            std::string longitude = latLongString.substr(spacePos + 1);

            GM_Point *newPoint = new GM_Point(std::stod(latitude), std::stod(longitude)); // note the order
            return newPoint;
        }
        if (fieldName == "surfaceProperty" || fieldName == "curveProperty")
        {
            auto position = findNodeRec(field, "posList");
            if (position == pugi::xml_node())
            {
                return nullptr;
            }
            std::string latLongString = position.text().get();

            std::vector<DirectPosition *> posList = parseCoordinates(latLongString);
            GM_Surface *newSurface = new GM_Surface(posList);
            return newSurface;
        }
    }
    return nullptr;
};
