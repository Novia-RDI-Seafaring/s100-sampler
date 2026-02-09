#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <SQLiteCpp/SQLiteCpp.h>
#include "S100Service.hpp"

class MSRDatabase
{
public:
    /*!
     * \brief Constructs and initializes the repository with a database path.
     * \param dbPath Path to the folder containing the SQLite database.
     */
    explicit MSRDatabase(const std::string &dbPath);

    /*!
     * \brief Inserts a new service to the database. Also updates the service endpoint if the service already exists.
     */
    bool insertService(const S100Service &service);

    /*!
     * \brief Updates the endpoint of a matching service
     */
    bool updateServiceEndpoint(const S100Service &service);

    /*!
     * \brief Updates the active status of a matching service
     */
    bool updateActiveStatus(const S100Service &service);

    /*!
     * \brief Returns all services in the local database
     */
    QVector<S100Service> getServices();

    /*!
     * \brief Returns all services of a provider in the local database
     */
    QVector<S100Service> getServicesOfProvider(S100Provider &provider);

    bool getActiveStatusFromDatabase(const S100Service &service);

    void clearTable();

private:
    std::unique_ptr<SQLite::Database> database; ///< Internal SQLite database handle
};
