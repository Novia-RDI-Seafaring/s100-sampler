#pragma once

#include "NAVWARNWrapper.hpp"
#include "NWParser.hpp"
#include "Curler.hpp"
#include <thread>
#include <SQLiteCpp/SQLiteCpp.h>
#include <unordered_set>
#include "Resources.hpp"
#include "S100Service.hpp"
#include "S100DataLayer.hpp"

/*!
 * \class NAVWARNDataLayer
 * \brief Manages NAVWARN message lifecycle: pulling, parsing, caching, and rendering coordination.
 *
 * Handles:
 * - Downloading S-124 NAVWARN messages from a remote service
 * - Parsing XML using NWParser
 * - Avoiding reprocessing via a SQLite cache
 * - Tracking render state using NAVWARNWrapper instances
 */
class NAVWARNDataLayer : public S100DataLayer
{
public:
    /*!
     * \brief Constructor. Initializes parser, folder paths, and database schema.
     */
    explicit NAVWARNDataLayer();

    ~NAVWARNDataLayer();

    /*!
     * \brief Scans XML folder for unprocessed files, parses them, and updates DB/cache.
     */
    void scanAndLoad() override;

    /*!
     * \brief Clears all NAVWARN entries from the SQLite database.
     */
    void clearDatabase() override;

private:
    std::unique_ptr<NWParser> parser;     ///< Parser used for S-124 NAVWARN XML
    /*!
     * \brief Loads filenames of already processed NAVWARNs from DB.
     */
    std::unordered_set<std::string> listFilesInDatabase();

    /*!
     * \brief Inserts a parsed NAVWARN record into the database.
     * \param nw Pointer to the parsed NAVWARNPreamble.
     * \param filename Original file path of the message.
     */
    void insertNWToDatabase(NAVWARNPreamble *nw, const std::string &filename);
};
