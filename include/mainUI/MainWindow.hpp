#pragma once

/**
 * \file MainWindow.hpp
 * \brief Declaration of the MainWindow class, the primary user interface controller of the ECS application.
 *
 * This class is responsible for:
 * - UI setup and window management
 * - Chart rendering and dataset handling
 * - Managing NAVWARN and ATON overlays and dialogs
 * - Receiving and processing NMEA and AIS ship data
 * - Handling toolbar actions, view modes, and data reloading
 *
 * The application uses Qt Widgets for GUI elements and integrates the Nautilus SDK for marine chart functionalities.
 *
 * ### Structure
 * - UI Initialization: `initializeUI`, `initializeToolbar`, `initializeDialogs`, etc.
 * - Chart Operations: `loadChart`, `clearCharts`, `addDataSetToChartWidget`, etc.
 * - Overlay Management: NAVWARN, ATON, OwnShip overlays and updates
 * - Event Handling: UI interactions, mouse events, view locking
 * - Data Sources: ATON/NW folders, NMEA input, ShipDataHandler
 * - External Dependencies: Nautilus SDK overlays and data layers
 *
 * ### Refactoring Note
 * This class is too large and violates the Single Responsibility Principle.
 * Suggested cleanup:
 * - Split overlays and data loading into separate managers
 * - Encapsulate chart-related behavior into a controller
 * - Remove unused and commented-out methods
 * - Replace raw pointers with smart pointers consistently
 * - Extract toolbar and menu setup to helper classes
 */

// Qt tools
#include <QMainWindow>
#include <QWidgetAction>
#include <QInputDialog>
#include <qtoolbutton.h>

// Main dialog windows
#include "datadialog.h"
#include "vtsdialog.h"
#include "MSRMainModule.hpp"

#include "getsummarydialog.h"

// Custom overlays and data handlers
#include "S100DataDisplay.hpp"

#include "NAVWARNDataLayer.hpp"

#include "ATONDataLayer.hpp"


namespace Ui
{
    class MainWindow;
}

/*!
 * \class MainWindow
 * \brief Main application window managing the ECS UI and chart interactions.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*!
     * \brief  Constructs the main window and initializes UI and chart components.
     */

    explicit MainWindow(QWidget *parent = nullptr);

    /// Deleted copy constructor.
    MainWindow(const MainWindow &) = delete;

    /// Deleted assignment operator.
    MainWindow &operator=(const MainWindow &) = delete;

    /*!
     * \brief Destructor that clears chart databases and shuts down chart widgets.
     */
    ~MainWindow();

    /*!
     * \brief Adds a dataset to the chart widget.
     * \param datasetPath Path to the dataset (as EnString).
     * \param layer Layer to load the dataset onto.
     */
    //void addDataSetToChartWidget(Nautilus::EnString datasetPath, int layer = 100);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onOption1(const QPoint &pos);
    void showContextMenu(const QPoint &pos);
    void applyScale(double min, double max);
    void scanNWFolder();
    void scanATONFolder();
    void jumpToCoordinates(DirectPosition *);
    void updateConning(double lat, double lon, double sog, double head);
    void onServiceChange(const S100Service &service);


private:
    void onClick(QMouseEvent *);

    /// Initializes window layout, toolbar, fonts, and dialogs
    void initializeUI();
    void initializeToolbar();
    void initializeChart();
    void initializeOverlays();
    void initializeShipSystems();
    void initializeDialogs();
    void initializeTimers();
    void initializeFontMenu();

    /// Chart loading and clearing
    void loadChart();
    void loadFullChartSet();
    void loadApproaches(int layer = 200);
    void loadGeneralCharts(int layer = 100);
    void loadDanishCharts(int layer);
    void loadTurkuHarbor(int layer = 300);
    void clearCharts();
    
    /// Chart rendering settings
    void changeColourPalette();
    //void changeViewGroup(Nautilus::Portrayal::EnDisplayMode::DisplayCategory);
    void pointNorth();
    void lockToShip();
    void updateLockToShipButtonStyle(bool locked);
    void moveToShip();

    /// Feature configuration
    void setupShipOverlay();
    void generateIcons();
    void showVTSDialog();
    void showNWDialog();
    void showATONDialog();
    void showMSRMenu();
    void showSummary();
    void fullATONReload();
    void fullNWReload();
    void deactivateVTS();

    /// Settings menus
    void showDisplayCategoryMenu();
    void changeLoadingStrategy();
    void changeAntialiasingMode();
    void changePreviewMode();
    void changePainterMode();
    void changeAsyncMode();

    //std::vector<Nautilus::EnString> encProductlist;
    std::unique_ptr<Ui::MainWindow> m_ui;

    VTSDialog *vtsDialogInstance = nullptr;
    std::unique_ptr<getSummaryDialog> getSummaryDisplay;
    std::unique_ptr<MSRMainModule> MSRMenuModule = nullptr;
    QFont defaultAppFont;

    std::unique_ptr<S100DataDisplay> nwDisplay;
    std::unique_ptr<S100DataDisplay> atonDisplay;

    // ShipDataHandler *shipDataHandler = nullptr;
    // NmeaProcessor *nmeaProcessor = nullptr;

    QTimer *frameTimer = nullptr;
    QTimer *nwTimer = nullptr;
    QTimer *atonTimer = nullptr;

    QMenuBar *mainMenuBar = nullptr;

    // std::unique_ptr<Nautilus::Navigational::EnScaleBarOverlay> scaleBarOverlay;
    // std::unique_ptr<Nautilus::OwnShip::EnOwnShipOverlay> ownShipOverlay;
    // std::unique_ptr<NAVWARNDynamicOverlay> navwarnOverlay;
    // std::unique_ptr<ATONDynamicOverlay> atonOverlay;

    //std::shared_ptr<Nautilus::DataRepository::EnChartLibrary> library;

    bool shipIsSetupped = false;
    bool viewPortLocked = false;
    bool originAtShip = false;

    /*!
     * \brief Reads file paths from a folder and converts them to EnString format.
     * \param folderPath Path to the folder.
     * \return List of EnStrings representing file paths.
     */
    //std::vector<Nautilus::EnString> getFilesInFolder(std::string folderPath);

    /*!
     * \brief Clears the VTS-message notification associated with the input parameter
     */
    void clearRelatedNotification(VTSDigitalInformationMessage *message);
};
