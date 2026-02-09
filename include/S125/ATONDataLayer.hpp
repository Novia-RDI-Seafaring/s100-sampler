#pragma once

#include "ATONWrapper.hpp"
#include "ATONParser.hpp"
#include "Curler.hpp"
#include <thread>
#include <SQLiteCpp/SQLiteCpp.h>
#include <unordered_set>
#include "Resources.hpp"
#include "S100Service.hpp"
#include "S100DataLayer.hpp"
/*!
 * \class ATONDataLayer
 * \brief Manages ATON (Aids to Navigation) data processing, parsing, storage, and rendering state.
 *
 * Responsible for:
 * - Pulling ATON messages from a remote endpoint
 * - Parsing S-125 GML/XML data
 * - Storing parsed objects into SQLite
 * - Tracking and exposing undrawn ATONs
 * - Coordinating rendering state via ATONWrapper objects
 */
class ATONDataLayer : public S100DataLayer
{
public:
    /*!
     * \brief Constructor. Initializes the parser and database tables if needed.
     */
    ATONDataLayer();

    ~ATONDataLayer();

    /*!
     * \brief Loads and parses ATON messages from local folder, skipping those already in DB.
     */
    void scanAndLoad() override;

    /*!
     * \brief Deletes all ATON records from the local SQLite database.
     */
    void clearDatabase() override;

private:
    std::unique_ptr<ATONParser> parser; ///< XML parser for ATONMember objects

    /*!
     * \brief Extracts the UUID from a file's GML root `gml:id` attribute.
     * Shouldn't be here.
     */
    std::string extractUUIDFromFile(const std::string &filePath);

    /*!
     * \brief Returns set of file names already processed (from DB).
     */
    std::unordered_set<std::string> getProcessedFilesFromDatabase();

    /*!
     * \brief Inserts parsed ATON objects and their statuses into the DB.
     * \param atonList List of ATONMember pointers parsed from XML.
     * \param filename Source file associated with the members.
     */
    void insertATONMembers(const std::vector<ATONMember *> &atonList, const std::string &filename);
};
