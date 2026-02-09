#pragma once

#include <QObject>
#include <memory>
#include "MSRUI.hpp"
#include "S100Service.hpp"
#include "MSRDataHandler.hpp"

/*!
 * \class MSRMainModule
 * \brief
 *
 * Responsible for:
 * - Holding the UI and DataHandler
 * - Transporting the Qt signals upstream
 */
class MSRMainModule : public QObject
{
    Q_OBJECT

public:
    explicit MSRMainModule(QObject *parent = nullptr);

    void showDialog();
    /*!
     * \brief Initializes the main module by loading the services from local database
     */
    void init();

signals:
    /*!
     * \brief Signal for service active change due to user input in the UI
     */
    void serviceChange(const S100Service &service);

private slots:
    void onProviderCall(S100Provider provider);
    void onServiceChange(const S100Service &service);
    void onPingSignal(const QUrl &endpoint);

    void onRestoreDefaults();

private:
    std::unique_ptr<MSRUI> m_ui;
    std::unique_ptr<MSRDataHandler> m_handler;
    QVector<S100Provider> m_providers;
};