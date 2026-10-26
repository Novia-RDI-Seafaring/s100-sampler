#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "S100Service.hpp"
#include "S100FeatureWrapper.hpp"
#include "Curler.hpp"
#include <SQLiteCpp/SQLiteCpp.h>
#include <QObject>
#include <Resources.hpp>
struct ServiceError
{
    std::string mainMessage;
    HttpResponse response;
    S100Service service;
};

/*!
 * \class S100DataLayer
 * \brief Abstract base class for handling S-100 series data layers (S-124, S-125).
 *
 * Defines the common interface for:
 * - Pulling and scanning datasets
 * - Parsing and inserting to DB
 * - Managing render state
 * - Coordinating services
 */
class S100DataLayer : public QObject
{

    Q_OBJECT

public:
    explicit S100DataLayer() {};

    virtual ~S100DataLayer() = default;

    /*!
     * \brief Downloads the full dataset from a remote source.
     */
    void pullAll()
    {
        const auto &cfg = ConfigManager::getInstance().getConfig();
        const auto &certOpt = cfg.cert;
        const auto &keyOpt = cfg.key;

        for (const auto service : serviceList)
        {
            std::string url = service.endpoint.toString().toStdString() + "/v1/object";
            HttpResponse response;
            try
            {
                response = Curler::getJson(url, certOpt, keyOpt);
                Curler::saveAndDecodeRawJSONToFiles(response.body, this->outputFolder);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Pull error: " << e.what() << std::endl;
                std::string errorMessage = e.what();
                ServiceError newError = {errorMessage, response, service};
                emit generalError(newError);
            }
        }
    };

    /*!
     * \brief Backs up all XML/GML files to a `backup` subdirectory.
     * \return True on success.
     */
    bool backUpData()
    {
        std::filesystem::path sourceDir = this->outputFolder;
        std::filesystem::path backupDir = this->outputFolder + "/backup";

        if (!std::filesystem::exists(backupDir))
        {
            std::filesystem::create_directories(backupDir);
        }
        if (!std::filesystem::exists(sourceDir))
        {
            std::cerr << "ERROR: Source directory does not exist: " << sourceDir << "\n";
            return false;
        }
        for (const auto &entry : std::filesystem::directory_iterator(sourceDir))
        {

            if (!entry.is_regular_file())
                continue;

            auto ext = entry.path().extension();
            if (ext != ".xml" && ext != ".gml")
                continue;

            std::filesystem::path target = backupDir / entry.path().filename();

            try
            {
                std::filesystem::rename(entry.path(), target); // Moves file
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to move " << entry.path() << ": " << e.what() << std::endl;
                return false;
            }
        }

        std::cout << "Backup completed.\n";
        return true;
    };

    /*!
     * \brief Returns a list of undrawn feature wrappers for rendering.
     */
    std::vector<S100FeatureWrapper *> getUndrawnFeatures()
    {
        std::vector<S100FeatureWrapper *> undrawnVector;

        for (const auto &entry : featureMap)
        {
            auto *wrappedFeature = entry.second;

            // If the ATONWrapper is valid and undrawn, add to the list
            if (wrappedFeature && !wrappedFeature->isDrawn()) // Assuming ATONWrapper has `isDrawn()`
            {
                undrawnVector.push_back(wrappedFeature);
            }
        }

        std::cout << "Retrieved " << undrawnVector.size() << " undrawn members from memory.\n";
        return undrawnVector;
    };

    /*!
     * \brief Marks a feature as drawn in memory (non-persistent).
     * \param featureID UUID of the feature.
     */
    void markFeatureAsDrawn(const std::string &featureID)
    {
        try
        {
            // Mark as drawn in memory
            if (featureMap.find(featureID) != featureMap.end() && featureMap[featureID])
            {
                featureMap[featureID]->updateDrawStatus(true);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Database update error: " << e.what() << std::endl;
        }
    };

    /*!
     * \brief Performs a full update:
     * - Reload local files when no remote services are selected
     * - Backup existing files
     * - Clear DB
     * - Pull new data
     * - Parse and insert
     */
    void fullRefresh()
    {
        std::cout << "Starting full refresh of data...\n";

        if (serviceList.empty())
        {
            std::cout << "No remote services selected; reloading local data.\n";
            clearDatabase();
            featureMap.clear();
            scanAndLoad();
            return;
        }

        auto bol = backUpData();

        clearDatabase();

        featureMap.clear();

        try
        {
            pullAll();
        }
        catch (const std::bad_optional_access &e)
        {
            HttpResponse emptyResponse{};
            S100Service emptyService{};

            ServiceError newError{
                std::string("Internal state error (missing value): ") + e.what(),
                emptyResponse,
                emptyService};

            emit generalError(newError);
            return;
        }
        catch (const std::runtime_error &e)
        {
            HttpResponse emptyResponse{};
            S100Service emptyService{};

            ServiceError newError{
                std::string("Pull ALL failed: ") + e.what(),
                emptyResponse,
                emptyService};

            emit generalError(newError);
            return;
        }
        catch (const std::exception &e)
        {
            HttpResponse emptyResponse{};
            S100Service emptyService{};

            ServiceError newError{
                std::string("Unexpected error during Pull ALL: ") + e.what(),
                emptyResponse,
                emptyService};

            emit generalError(newError);
            return;
        }

        try
        {
            scanAndLoad();
        }
        catch (const std::runtime_error &e)
        {
            HttpResponse emptyResponse{};
            S100Service emptyService{};

            ServiceError newError{
                std::string("Scan failed: ") + e.what(),
                emptyResponse,
                emptyService};

            emit generalError(newError);
            return;
        }
        catch (const std::exception &e)
        {
            HttpResponse emptyResponse{};
            S100Service emptyService{};

            ServiceError newError{
                std::string("Unexpected error during scan: ") + e.what(),
                emptyResponse,
                emptyService};

            emit generalError(newError);
            return;
        }
    };

    /*!
     * \brief Adds a service to the active list. Skips duplicates.
     */
    void addService(const S100Service &service)
    {
        std::cout << "here: " << service.active << '\n';

        auto it = std::find(serviceList.begin(), serviceList.end(), service);
        if (it == serviceList.end())
        {
            serviceList.push_back(service);
            std::cout << "Updated service list with " << serviceList.size() << " services.\n";
        }
    };

    /*!
     * \brief Removes a service from the active list.
     */
    void removeService(const S100Service &service)
    {
        std::cout << "here: " << service.active << '\n';

        auto it = std::find(serviceList.begin(), serviceList.end(), service);
        if (it != serviceList.end())
        {
            serviceList.erase(it);
            std::cout << "Removed service. Service list now has " << serviceList.size() << " entries.\n";
        }
        else
        {
            std::cout << "Service not found. No removal performed.\n";
        }
    };

    /*!
     * \brief Replaces the current service list.
     */
    void updateServiceList(const std::vector<S100Service> &services)
    {
        serviceList = services;
        std::cout << "Updated service list with " << serviceList.size() << " services.\n";
    };

    /*!
     * \brief Returns list of XML/GML files found in the given folder.
     */
    std::vector<std::string> listXMLFilesInFolder(const std::string &folderPath)
    {
        std::vector<std::string> files;
        // Use filesystem library to iterate through files in the folder
        for (const auto &entry : std::filesystem::directory_iterator(folderPath))
        {
            if (entry.path().extension() == ".xml" || entry.path().extension() == ".gml")
            {
                files.push_back(entry.path().string());
            }
        }
        return files;
    }

    // Virtuals

    /*!
     * \brief Scans the specified folder for unprocessed files and loads them into DB.
     */
    virtual void scanAndLoad() = 0;

    /*!
     * \brief Deletes all records from the local SQLite database.
     */
    virtual void clearDatabase() = 0;

signals:
    void generalError(ServiceError);

protected:
    std::string outputFolder;           ///< Folder where XML/GML files are stored
    std::filesystem::path databasePath; ///< Path to the SQLite database file

    std::vector<S100Service> serviceList;
    std::unordered_map<std::string, S100FeatureWrapper *> featureMap; ///< UUID -> FeatureWrapper
    std::unique_ptr<SQLite::Database> database;                       ///< Internal SQLite database handle
};
