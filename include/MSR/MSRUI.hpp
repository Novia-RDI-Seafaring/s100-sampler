#pragma once

#include <QDialog>
#include <QTreeWidget>
#include <QPushButton>
#include "S100Service.hpp"
#include <QToolButton>
namespace Ui
{
    class MSRUI;
}

class MSRUI : public QDialog
{
    Q_OBJECT

public:
    explicit MSRUI(QWidget *parent = nullptr);
    ~MSRUI();
    /**
     * @brief Loads a list of S100Service objects into the MSRUI.
     *
     * The loadServices function populates the user interface with a list of S100Service objects.
     * @param services A QVector of S100Service objects.
     *
     */
    void loadServices(QVector<S100Service> services);
    
    void loadProviders(QVector<S100Provider> providers);
    void updatePingStatus(const QUrl &endpoint, bool);
signals:
    void providerCall(S100Provider provider);
    void serviceChange(S100Service service);
    void pingSignal(const QUrl &endpoint);
    void clearDatabase();
private slots:
    void onServiceCheckboxStateChanged(QTreeWidgetItem *item, int column);
    void onPingClicked();
    void onRestoreDefaults();
private:

    Ui::MSRUI *ui;
    void populateTempData();
};