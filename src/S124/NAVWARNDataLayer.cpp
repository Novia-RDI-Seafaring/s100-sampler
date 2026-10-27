#include "NAVWARNDataLayer.hpp"

NAVWARNDataLayer::NAVWARNDataLayer() : S100DataLayer()
{
    parser = std::make_unique<NWParser>();

    outputFolder = "../data/NW/";
    auto dbDir = std::filesystem::path("../data/DB/NW.db").parent_path();
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
        database = std::make_unique<SQLite::Database>("../data/DB/NW.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        // Create NW Members table
        database->exec("CREATE TABLE IF NOT EXISTS naval_warnings ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "member_id TEXT NOT NULL UNIQUE, "
                       "filename TEXT NOT NULL);");

        std::cout << "NW Database initialized successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database error: " << e.what() << " (errno=" << errno << ")\n";
    }
}
NAVWARNDataLayer::~NAVWARNDataLayer()
{
    std::cout << "Destroying NAVWARNDataLayer: this=" << this << "\n";
}
void NAVWARNDataLayer::clearDatabase()
{
    try
    {
        // Delete all records from tables
        database->exec("DELETE FROM naval_warnings;");

        std::cout << "NW Database cleared successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "NW Database clearing error: " << e.what() << std::endl;
    }
}

std::unordered_set<std::string> NAVWARNDataLayer::listFilesInDatabase()
{
    std::unordered_set<std::string> processedFiles;
    try
    {
        SQLite::Statement query(*database, "SELECT DISTINCT filename FROM naval_warnings;");

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

void NAVWARNDataLayer::scanAndLoad()
{
    // List all XML files in the folder
    featureMap.clear();
    std::vector<std::string> xmlFiles = listXMLFilesInFolder(outputFolder);
    std::unordered_set<std::string> processedFiles = listFilesInDatabase();

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
                        std::cout << "Starting reading XML" << '\n';

            NAVWARNPreamble *preamble = parser->readMembersFromXML(doc);
                                    std::cout << "XML read" << '\n';

            if (!preamble)
            {
                std::cout << "No valid NW preamble parsed in " << file << std::endl;
                continue;
            }
            std::cout << "We got here??" << '\n';
            // Store the new members in the database
            if (!alreadyStored)
                insertNWToDatabase(preamble, file);
            auto newNWWrap = new NAVWARNWrapper(preamble);
            featureMap[preamble->getGMLID()] = newNWWrap;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Error processing file " << file << ": " << ex.what() << std::endl;
        }
    }
}

void NAVWARNDataLayer::insertNWToDatabase(NAVWARNPreamble *nw, const std::string &filename)
{
    try
    {
        // Insert NW Member
        SQLite::Statement memberQuery(*database, "INSERT INTO naval_warnings (member_id, filename) VALUES (?, ?);");

        std::string memberID = nw->getGMLID();

        // Insert member into `naval_warnings` table
        memberQuery.bind(1, memberID);
        memberQuery.bind(2, filename);
        memberQuery.exec();
        memberQuery.reset(); // Reset for next NW Member
        //std::cout << "Inserted NW member " << filename << ".\n";
    }

    catch (const std::exception &e)
    {
        std::cerr << "NW Database insertion error: " << e.what() << std::endl;
    }
}