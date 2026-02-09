#include "MSRMainModule.hpp"

MSRMainModule::MSRMainModule(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<S100Service>("S100Service");
    qRegisterMetaType<QVector<S100Service>>("QVector<S100Service>");
    m_ui = std::make_unique<MSRUI>();
    m_handler = std::make_unique<MSRDataHandler>();

    // Default public MSR/SECOM providers.
    // These are demonstrator / public endpoints.
    // If different providers are needed, this should be made configurable.
    S100Provider p1{"Fintraffic", "Fintraffic", QUrl("https://msr.services-test.enav.fi"), 2};
    S100Provider p2{"Public Demonstrator MCP", "PDMCP", QUrl("https://msr.maritimeconnectivity.net/api/secom"), 1};

    m_providers.push_back(p1);
    m_providers.push_back(p2);

    m_ui->loadProviders(m_providers);

    connect(m_ui.get(), &MSRUI::providerCall, this, &MSRMainModule::onProviderCall);
    connect(m_ui.get(), &MSRUI::serviceChange, this, &MSRMainModule::onServiceChange);
    connect(m_ui.get(), &MSRUI::pingSignal, this, &MSRMainModule::onPingSignal);
    connect(m_ui.get(), &MSRUI::clearDatabase, this, &MSRMainModule::onRestoreDefaults);
}

void MSRMainModule::showDialog()
{
    m_ui->show();
}

void MSRMainModule::init()
{
    m_ui->loadServices(m_handler->getServicesFromDatabase());
}

void MSRMainModule::onProviderCall(S100Provider provider)
{
    m_handler->updateServiceList(provider);
    auto services = m_handler->getCurrentServices(provider);
    m_ui->loadServices(services);
}

void MSRMainModule::onServiceChange(const S100Service &service)
{
    m_handler->updateActiveStatus(service); // Flows to the database
                                            // bool status = m_handler->getActiveStatusFromDatabase(service);
    // std::cout << "Service active in MainModule?: " << service.active << '\n';

    // if (service.active == status) // All good, flow forward
    // {
    emit(serviceChange(service)); // Flows up to MainUI
    // }
    // else
    // {
    //     std::cerr << "Mismatch after DB write. Input: " << service.active
    //               << ", DB: " << status << '\n';
    // }
}

void MSRMainModule::onPingSignal(const QUrl &endpoint)
{
    bool response = m_handler->pingEndpoint(endpoint);
    m_ui->updatePingStatus(endpoint, response);
}

void MSRMainModule::onRestoreDefaults()
{
    m_handler->clearDatabase();
}

#include "moc_MSRMainModule.cpp"
