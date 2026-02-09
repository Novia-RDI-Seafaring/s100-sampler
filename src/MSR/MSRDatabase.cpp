#include "MSRDatabase.hpp"

MSRDatabase::MSRDatabase(const std::string &dbPath)
{
    std::cout << "MSR database creation" << '\n';

    try
    {
        database = std::make_unique<SQLite::Database>("../data/DB/msr.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        // Create messages table if it doesn't exist
        database->exec("CREATE TABLE IF NOT EXISTS msr ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "providerOrgId TEXT, "
                       "orgId TEXT, "
                       "name TEXT, "
                       "dataProductType TEXT, "
                       "endpoint TEXT, "
                       "active BOOLEAN DEFAULT 0);");

        std::cout << "MSR database created ok" << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
}

QVector<S100Service> MSRDatabase::getServicesOfProvider(S100Provider &provider)
{
    QVector<S100Service> services;

    try
    {
        SQLite::Statement query(*database,
                                "SELECT providerOrgId, orgId, name, dataProductType, endpoint, active "
                                "FROM msr WHERE providerOrgId = ?;");

        // Convert QString to std::string
        query.bind(1, provider.orgId.toStdString());

        while (query.executeStep())
        {
            S100Service service;
            service.providerOrgId = QString::fromStdString(query.getColumn(0).getString());
            service.orgId = QString::fromStdString(query.getColumn(1).getString());
            service.name = QString::fromStdString(query.getColumn(2).getString());
            service.dataProductType = QString::fromStdString(query.getColumn(3).getString());
            service.endpoint = QUrl(QString::fromStdString(query.getColumn(4).getString()));
            service.active = bool(query.getColumn(5).getInt());

            services.append(service);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database query error: " << e.what() << std::endl;
    }

    return services;
}

bool MSRDatabase::insertService(const S100Service &service)
{

    try
    {
        updateServiceEndpoint(service);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database update error: " << e.what() << std::endl;
        return false;
    }

    try
    {
        // Check if service already exists
        SQLite::Statement check(*database,
                                "SELECT COUNT(*) FROM msr "
                                "WHERE orgId = ? AND name = ? AND dataProductType = ? "
                                "AND endpoint = ?");

        check.bind(1, service.orgId.toStdString());
        check.bind(2, service.name.toStdString());
        check.bind(3, service.dataProductType.toStdString());
        check.bind(4, service.endpoint.toString().toStdString());

        if (check.executeStep() && check.getColumn(0).getInt() > 0)
        {
            return false; // Already exists
        }

        // Insert new service
        SQLite::Statement insert(*database,
                                 "INSERT INTO msr (providerOrgId, orgId, name, dataProductType, endpoint, active) "
                                 "VALUES (?, ?, ?, ?, ?, ?)");

        insert.bind(1, service.providerOrgId.toStdString());
        insert.bind(2, service.orgId.toStdString());
        insert.bind(3, service.name.toStdString());
        insert.bind(4, service.dataProductType.toStdString());
        insert.bind(5, service.endpoint.toString().toStdString());
        insert.bind(6, service.active);
        insert.exec();
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Insert error: " << e.what() << std::endl;
        return false;
    }
}

bool MSRDatabase::updateServiceEndpoint(const S100Service &service)
{
    try
    {
        // Check if service exists
        SQLite::Statement check(*database,
                                "SELECT COUNT(*) FROM msr "
                                "WHERE orgId = ? AND name = ? AND dataProductType = ? "
                                "AND active = ?");

        check.bind(1, service.orgId.toStdString());
        check.bind(2, service.name.toStdString());
        check.bind(3, service.dataProductType.toStdString());
        check.bind(4, 1); // assuming active is true

        if (check.executeStep() && check.getColumn(0).getInt() == 0)
        {
            return false; // Service does not exist
        }

        // Update the endpoint
        SQLite::Statement update(*database,
                                 "UPDATE msr SET endpoint = ? "
                                 "WHERE orgId = ? AND name = ? AND dataProductType = ? "
                                 "AND active = ?");

        update.bind(1, service.endpoint.toString().toStdString());
        update.bind(2, service.orgId.toStdString());
        update.bind(3, service.name.toStdString());
        update.bind(4, service.dataProductType.toStdString());
        update.bind(5, 1); // assuming active is true

        update.exec();
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Update error: " << e.what() << std::endl;
        return false;
    }
}

bool MSRDatabase::updateActiveStatus(const S100Service &service)
{

    try
    {
        // Check if service exists
        SQLite::Statement check(*database,
                                "SELECT COUNT(*) FROM msr "
                                "WHERE orgId = ? AND name = ? AND dataProductType = ? ");

        check.bind(1, service.orgId.toStdString());
        check.bind(2, service.name.toStdString());
        check.bind(3, service.dataProductType.toStdString());

        if (check.executeStep() && check.getColumn(0).getInt() == 0)
        {
            return false; // Service does not exist
        }

        // Update the endpoint
        SQLite::Statement update(*database,
                                 "UPDATE msr SET active = ? "
                                 "WHERE orgId = ? AND name = ? AND dataProductType = ? "
                                 "AND endpoint = ?");
        update.bind(1, service.active);
        update.bind(2, service.orgId.toStdString());
        update.bind(3, service.name.toStdString());
        update.bind(4, service.dataProductType.toStdString());
        update.bind(5, service.endpoint.toString().toStdString());

        update.exec();
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Update error: " << e.what() << std::endl;
        return false;
    }
}

QVector<S100Service> MSRDatabase::getServices()
{
    QVector<S100Service> services;
    try
    {
        SQLite::Statement query(*database,
                                "SELECT providerOrgId, orgId, name, dataProductType, endpoint, active "
                                "FROM msr;");

        while (query.executeStep())
        {
            S100Service service;
            service.providerOrgId = QString::fromStdString(query.getColumn(0).getString());
            service.orgId = QString::fromStdString(query.getColumn(1).getString());
            service.name = QString::fromStdString(query.getColumn(2).getString());
            service.dataProductType = QString::fromStdString(query.getColumn(3).getString());
            service.endpoint = QUrl(QString::fromStdString(query.getColumn(4).getString()));
            service.active = bool(query.getColumn(5).getInt());

            services.append(service);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database query error: " << e.what() << std::endl;
    }

    return services;
}

bool MSRDatabase::getActiveStatusFromDatabase(const S100Service &service)
{
    try
    {
        SQLite::Statement query(*database,
                                "SELECT active "
                                "FROM msr WHERE orgId = ? AND name = ? AND dataProductType = ? AND endpoint = ?;");

        query.bind(1, service.orgId.toStdString());
        query.bind(2, service.name.toStdString());
        query.bind(3, service.dataProductType.toStdString());
        query.bind(4, service.endpoint.toString().toStdString());

        if (query.executeStep())
        {
            return query.getColumn(0).getInt() != 0;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database query error: " << e.what() << std::endl;
    }

    return false;
}
void MSRDatabase::clearTable()
{
    database->exec("DELETE FROM msr;");
}