#include "MSRDataHandler.hpp"

MSRDataHandler::MSRDataHandler(QObject *parent)
    : QObject(parent)
{
    std::filesystem::path dbp = "../data/DB/";
    m_curler = std::make_unique<MSRCurler>();
    m_db = std::make_unique<MSRDatabase>(dbp.string());
}

QVector<S100Service> MSRDataHandler::getServiceList(S100Provider provider)
{
    json jsonResponse;
    // if (provider.name == "Fintraffic")
    // {
    //     jsonResponse = m_curler->getFintraffic();
    // }
    // else
    // {
    //     jsonResponse = m_curler->getMCP();
    // }

    jsonResponse = m_curler->getServiceList(provider, provider.secomVersion);
    return parseSearchResultsToVector(provider, jsonResponse);
}

bool MSRDataHandler::updateServiceList(S100Provider provider)
{
    auto serviceList = getServiceList(provider);
    bool updated = false;
    for (auto service : serviceList)
    {
        bool update = m_db->insertService(service);
        if (update)
        {
            updated = true;
        }
    }
    return updated;
}

QVector<S100Service> MSRDataHandler::getCurrentServices(S100Provider provider)
{
    return m_db->getServicesOfProvider(provider);
}

QVector<S100Service> MSRDataHandler::getServicesFromDatabase(){
    return m_db->getServices();
};

QVector<S100Service> MSRDataHandler::parseSearchResultsToVector(const S100Provider &provider, const json &response)
{
    QVector<S100Service> services;

    if (!response.contains("searchServiceResult") || !response["searchServiceResult"].is_array())
    {
        return services;
    }

    for (const auto &result : response["searchServiceResult"])
    {
        QString name = QString::fromStdString(result.value("name", ""));
        QString organization = QString::fromStdString(result.value("organizationId", ""));
        QString dataProductType = QString::fromStdString(result.value("dataProductType", ""));

        QString endpointStr = QString::fromStdString(result.value("endpointUri", "")).trimmed();
        QUrl endpoint(endpointStr);

        if (!endpoint.isValid())
        {
            std::cerr << "Not valid endpoint in " << name.toStdString() << '\n';
             std::cerr << "Endpoint is " << endpointStr.toStdString() << '\n';

            continue;
        }

        S100Service service{provider.orgId, organization, name, dataProductType, endpoint, false};
        services.append(service);
    }

    return services;
}

void MSRDataHandler::updateActiveStatus(const S100Service &service)
{
    m_db->updateActiveStatus(service);
}

bool MSRDataHandler::getActiveStatusFromDatabase(const S100Service &service)
{
    return m_db->getActiveStatusFromDatabase(service);
}

bool MSRDataHandler::pingEndpoint(const QUrl endpoint)
{
    return m_curler->pingEndpoint(endpoint);
}

void MSRDataHandler::clearDatabase(){
    m_db->clearTable();
}

#include "moc_MSRDataHandler.cpp"
