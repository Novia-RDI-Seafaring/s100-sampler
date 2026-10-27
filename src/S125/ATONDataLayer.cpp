#include "ATONDataLayer.hpp"

ATONDataLayer::ATONDataLayer() : S100DataLayer()
{
    parser = std::make_unique<ATONParser>();
    outputFolder = "../data/AtoN/";

    auto dbDir = std::filesystem::path("../data/DB/aton.db").parent_path();
    auto dataDir = std::filesystem::path(outputFolder);

    if (!std::filesystem::exists(dbDir))
    {
        std::cout << "DB directory missing: " << dbDir << "\n";
        std::filesystem::create_directories(dbDir);
    }

    if (!std::filesystem::exists(dataDir))
    {
        std::cout << "Data directory missing: " << dataDir << "\n";
        std::filesystem::create_directories(dataDir);
    }

    try
    {
        database = std::make_unique<SQLite::Database>("../data/DB/aton.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        // Create ATON Members table
        database->exec("CREATE TABLE IF NOT EXISTS aton_members ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "member_id TEXT NOT NULL UNIQUE, "
                       "filename TEXT NOT NULL);");

        // Create ATON Statuses table (linked to ATON Members)
        database->exec("CREATE TABLE IF NOT EXISTS aton_statuses ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "member_id TEXT NOT NULL, "
                       "status_href TEXT NOT NULL, "
                       "FOREIGN KEY(member_id) REFERENCES aton_members(member_id));");

        std::cout << "Database initialized successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database error: " << e.what() << " (errno=" << errno << ")\n";
    }
}

ATONDataLayer::~ATONDataLayer()
{
    std::cout << "Destroying ATONDataLayer: this=" << this << "\n";
}

std::unordered_set<std::string> ATONDataLayer::getProcessedFilesFromDatabase()
{
    std::unordered_set<std::string> processedFiles;
    try
    {
        SQLite::Statement query(*database, "SELECT DISTINCT filename FROM aton_members;");

        while (query.executeStep())
        {
            processedFiles.insert(query.getColumn(0).getString());
        }
        std::cout << "Loaded " << processedFiles.size() << " already processed files from database.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database retrieval error: " << e.what() << std::endl;
    }
    return processedFiles;
}

void ATONDataLayer::scanAndLoad()
{
    featureMap.clear();
    // List all XML files in the folder
    std::vector<std::string> xmlFiles = listXMLFilesInFolder(outputFolder);
    std::unordered_set<std::string> processedFiles = getProcessedFilesFromDatabase(); // Retrieve already stored filenames

    for (const auto &file : xmlFiles)
    {
        try
        {
            // Files already in the database are parsed again for display, but not inserted twice
            const bool alreadyStored = processedFiles.find(file) != processedFiles.end();

            // Load the XML document
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file(file.c_str());
            if (!result)
            {
                throw std::runtime_error("Failed to parse XML file: " + file);
            }

            // Read members using the parser
            std::vector<ATONMember *> members = parser->readMembersFromXML(doc);
            if (members.empty())
            {
                continue;
            }

            // Store the new members in the database
            if (!alreadyStored)
                insertATONMembers(members, file);
            for (auto atonMember : members)
            {
                auto atonWRAP = new ATONWrapper(atonMember);
                featureMap[atonMember->getGMLID()] = atonWRAP;
            }
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Error processing file " << file << ": " << ex.what() << std::endl;
        }
    }
}

void ATONDataLayer::insertATONMembers(const std::vector<ATONMember *> &atonList, const std::string &filename)
{
    try
    {
        // Insert ATON Member
        SQLite::Statement memberQuery(*database, "INSERT INTO aton_members (member_id, filename) VALUES (?, ?);");

        // Insert ATON Status (linked to ATON Member)
        SQLite::Statement statusQuery(*database, "INSERT INTO aton_statuses (member_id, status_href) VALUES (?, ?);");

        for (const auto *aton : atonList)
        {
            std::string memberID = aton->getGMLID();

            // Insert member into `aton_members` table
            memberQuery.bind(1, memberID);
            memberQuery.bind(2, filename);
            memberQuery.exec();
            memberQuery.reset(); // Reset for next ATON Member

            // Insert all statuses for this ATON Member
            auto statusList = aton->getStatusList(); // Assuming this returns a vector<Status*>
            for (const auto *status : statusList)
            {
                //std::cout << "Binding status href " << status->href << " with " << memberID << '\n';
                statusQuery.bind(1, memberID);
                statusQuery.bind(2, status->href);

                statusQuery.exec();
                statusQuery.reset(); // Reset for next status
            }
        }
        //std::cout << "Inserted " << atonList.size() << " ATON members with statuses into database from " << filename << ".\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database insertion error: " << e.what() << std::endl;
    }
}

std::string ATONDataLayer::extractUUIDFromFile(const std::string &filePath)
{
    // Load the XML document using pugixml
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filePath.c_str());

    if (!result)
    {
        // Handle error if file cannot be parsed
        throw std::runtime_error("Failed to parse XML file: " + filePath);
    }

    // Access the root node
    pugi::xml_node root = doc.first_child();
    if (!root)
    {
        throw std::runtime_error("Invalid XML structure: missing root node in file " + filePath);
    }

    // Extract the gml:id attribute
    const char *gmlId = root.attribute("gml:id").value();
    if (!gmlId)
    {
        throw std::runtime_error("Missing gml:id attribute in file " + filePath);
    }

    // Check and remove the "id-" prefix, if it exists
    std::string uuid;
    if (std::strncmp(gmlId, "id-", 3) == 0)
    {
        uuid = std::string(gmlId + 3); // Skip the "id-" part
    }
    else
    {
        uuid = std::string(gmlId);
    }

    return uuid;
}

void ATONDataLayer::clearDatabase()
{
    try
    {
        // Delete all records from tables
        database->exec("DELETE FROM aton_statuses;");
        database->exec("DELETE FROM aton_members;");

        std::cout << "Database cleared successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database clearing error: " << e.what() << std::endl;
    }
}