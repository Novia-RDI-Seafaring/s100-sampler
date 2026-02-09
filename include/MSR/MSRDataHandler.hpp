#pragma once

#include <QObject>
#include <memory>
#include "MSRCurler.hpp"
#include "S100Service.hpp"
#include "MSRDatabase.hpp"

using json = nlohmann::json;

/*!
 * \class MSRDataHandler
 * \brief
 * Responsible for:
 * - Calling the Curl and downloading service lists
 * - Calling the DB interface
 */
class MSRDataHandler : public QObject
{
    Q_OBJECT

public:
    explicit MSRDataHandler(QObject *parent = nullptr);

    QVector<S100Service> getServiceList(S100Provider provider);
    QVector<S100Service> getCurrentServices(S100Provider provider);
    
    /*!
     * \brief Gets the all services from local database
     */
    QVector<S100Service> getServicesFromDatabase();

    /*!
     * \brief Gets the serviceList from MSR endpoint and updates the local database.
     * \return True if changes happened
     */
    bool updateServiceList(S100Provider provider);

    /*!
     * \brief Updates the active status in the DB according to the passed service
     */
    void updateActiveStatus(const S100Service &service);

    bool getActiveStatusFromDatabase(const S100Service &service);
    bool pingEndpoint(const QUrl endpoint);
    void clearDatabase();

private:
    std::unique_ptr<MSRCurler> m_curler;
    std::unique_ptr<MSRDatabase> m_db;

    QVector<S100Service> parseSearchResultsToVector(const S100Provider &provider, const json &response);
};