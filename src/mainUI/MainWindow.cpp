// MainWindow.cpp

#include "MainWindow.hpp"
#include "ui_MainWindow.h"
// Constructor

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(std::make_unique<Ui::MainWindow>())
{
    initializeUI();
    initializeToolbar();

    try
    {
        initializeChart();
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception caught:" << e.what();
    }
    try
    {
        initializeOverlays();
        initializeTimers();
        initializeDialogs();
        initializeShipSystems();
        initializeFontMenu();
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception caught:" << e.what();
    }
    qDebug() << " MW constructed" << '\n';

    const auto &cfg = ConfigManager::getInstance().getConfig();

    if (!cfg.cert.has_value() || !cfg.key.has_value())
    {
        QMessageBox::warning(
            nullptr,
            "Some network features disabled",
            "TLS certificates are not configured.\n\n"
            "Some network features will not be available until valid "
            "certificate and key paths are provided in the configuration file.");

        return;
    }

}

MainWindow::~MainWindow()
{
    atonDisplay->getDataLayer()->clearDatabase();
    nwDisplay->getDataLayer()->clearDatabase();
    //     m_ui->chartWidget->Shutdown();
}

void MainWindow::initializeUI()
{
    m_ui->setupUi(this);
    // setWindowState(Qt::WindowFullScreen);

    int padding = 10; // Add some padding from the edges if desired
                      //     m_ui->conningBox->move(m_ui->chartWidget->width() - m_ui->conningBox->width() - padding, padding);

    // Position notificationBox in the bottom-right corner
    //     m_ui->notificationBox->move(m_ui->chartWidget->width() - m_ui->notificationBox->width() - padding,
    //                                 m_ui->chartWidget->height() - m_ui->notificationBox->height() - padding);

    // Set transparency
    m_ui->conningBox->setWindowOpacity(0.7);
    m_ui->notificationBox->setWindowOpacity(0.7);

    mainMenuBar = menuBar();

    // Set shutdown menu
    QMenu *mainMenu = new QMenu("Menu", this);
    QAction *shutdownAction = new QAction("Close ECS", this);
    QAction *openMSRAction = new QAction("MSR Menu", this);
    mainMenu->addAction(openMSRAction);

    QAction *openSummaryAction = new QAction("AtoN Summaries", this);
    // mainMenu->addAction(openSummaryAction);

    mainMenu->addAction(shutdownAction);

    mainMenuBar->addMenu(mainMenu);

    connect(shutdownAction, &QAction::triggered, this, &MainWindow::close);
    connect(openMSRAction, &QAction::triggered, this, &MainWindow::showMSRMenu);
    // connect(openSummaryAction, &QAction::triggered, this, &MainWindow::showSummary);

    // Set chart loading menu
    QMenu *chartMenu = new QMenu("Charts", this);

    QAction *chartLoadingAction = new QAction("Load a New Chart", this);
    QAction *chartDeletionAction = new QAction("Clear Charts", this);
    QAction *approachesLoadingAction = new QAction("Load All Approaches", this);
    QAction *archipelagoLoadingAction = new QAction("Load Turku Archipelago", this);
    QAction *danishLoadingAction = new QAction("Load Danish charts", this);

    chartMenu->addAction(chartLoadingAction);
    chartMenu->addAction(archipelagoLoadingAction);
    chartMenu->addAction(approachesLoadingAction);
    chartMenu->addAction(danishLoadingAction);

    chartMenu->addAction(chartDeletionAction);

    mainMenuBar->addMenu(chartMenu);

    connect(chartLoadingAction, &QAction::triggered, this, &MainWindow::loadChart);
    connect(chartDeletionAction, &QAction::triggered, this, &MainWindow::clearCharts);
    connect(approachesLoadingAction, &QAction::triggered, this, &MainWindow::loadApproaches);
    connect(archipelagoLoadingAction, &QAction::triggered, this, &MainWindow::loadFullChartSet);
    connect(danishLoadingAction, &QAction::triggered, this, &MainWindow::loadDanishCharts);

    // Set chart settings menu
    QMenu *settingsMenu = new QMenu("Settings", this);

    QAction *painterAction = new QAction("Painter Mode", this);
    QAction *loadingAction = new QAction("Loading Strategy", this);
    QAction *antialiasingAction = new QAction("Antialiasing Mode", this);
    QAction *previewAction = new QAction("Preview Mode", this);
    QAction *asyncLoadingMode = new QAction("Async loading Mode", this);

    settingsMenu->addAction(painterAction);
    settingsMenu->addAction(loadingAction);
    settingsMenu->addAction(antialiasingAction);
    settingsMenu->addAction(previewAction);
    settingsMenu->addAction(asyncLoadingMode);

    mainMenuBar->addMenu(settingsMenu);

    connect(painterAction, &QAction::triggered, this, &MainWindow::changePainterMode);
    connect(loadingAction, &QAction::triggered, this, &MainWindow::changeLoadingStrategy);
    connect(antialiasingAction, &QAction::triggered, this, &MainWindow::changeAntialiasingMode);
    connect(previewAction, &QAction::triggered, this, &MainWindow::changePreviewMode);
    connect(asyncLoadingMode, &QAction::triggered, this, &MainWindow::changeAsyncMode);
}

void MainWindow::initializeOverlays()
{
    // routeOverlay = m_ui->chartWidget->CreateOverlay<Nautilus::Route::EnRouteOverlay>();
    // scaleBarOverlay = m_ui->chartWidget->CreateOverlay<Nautilus::Navigational::EnScaleBarOverlay>();

    // route = std::make_shared<Nautilus::Route::EnRoute>("Test Route");
    // navTools = new NavTools(m_ui->chartWidget);
    // try
    // {
    //     navwarnOverlay = m_ui->chartWidget->CreateOverlay<NAVWARNDynamicOverlay>();
    //     navwarnOverlay->SetLayer(1000);
    // }
    // catch (const Nautilus::EnException &ex)
    // {
    //     QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
    //     QMessageBox::critical(nullptr, "", msg);
    // }

    // try
    // {
    //     atonOverlay = m_ui->chartWidget->CreateOverlay<ATONDynamicOverlay>();
    //     atonOverlay->SetLayer(1000);
    // }
    // catch (const Nautilus::EnException &ex)
    // {
    //     QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
    //     QMessageBox::critical(nullptr, "", msg);
    // }
}

void MainWindow::initializeToolbar()
{
    generateIcons();

    // Pushes next widget to the right
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_ui->bottomToolbar->addWidget(spacer);

    QLabel *clockLabel = new QLabel("00:00", this);
    QWidgetAction *clockAction = new QWidgetAction(this);
    clockAction->setDefaultWidget(clockLabel);
    m_ui->bottomToolbar->addAction(clockAction);

    QWidget *pixelSpacer = new QWidget(this);
    pixelSpacer->setFixedWidth(10); // or whatever width you need
    pixelSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_ui->bottomToolbar->addWidget(pixelSpacer);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]()
            { clockLabel->setText(QTime::currentTime().toString("hh:mm:ss")); });
    timer->start(1000);

    connect(m_ui->actionDayNightMode, &QAction::triggered, this, &MainWindow::changeColourPalette);
    connect(m_ui->actionDisplayMode, &QAction::triggered, this, &MainWindow::showDisplayCategoryMenu);

    connect(m_ui->actionNorthUp, &QAction::triggered, this, &MainWindow::pointNorth);
    connect(m_ui->actionLockToShip, &QAction::triggered, this, &MainWindow::lockToShip);

    connect(m_ui->actionVTSMenu, &QAction::triggered, this, &MainWindow::showVTSDialog);
    connect(m_ui->actionNWMenu, &QAction::triggered, this, &MainWindow::showNWDialog);
    connect(m_ui->actionATONMenu, &QAction::triggered, this, &MainWindow::showATONDialog);
}

void MainWindow::initializeDialogs()
{
    auto layer = std::make_unique<NAVWARNDataLayer>();
    auto dialog = std::make_unique<DataDialog>(this);
    nwDisplay = std::make_unique<S100DataDisplay>(
        std::move(layer), std::move(dialog),
        "Navigational Warnings", "Manual NW update");

    connect(nwDisplay.get(), &S100DataDisplay::updateButtonClicked,
            this, &MainWindow::fullNWReload);

    connect(nwDisplay.get(), &S100DataDisplay::dataItemClicked,
            this, &MainWindow::jumpToCoordinates);

    atonDisplay = std::make_unique<S100DataDisplay>(
        std::make_unique<ATONDataLayer>(),
        std::make_unique<DataDialog>(this), "Changes to Aids to Navigation", "Manual AtoN update");

    connect(atonDisplay.get(), &S100DataDisplay::updateButtonClicked,
            this, &MainWindow::fullATONReload);

    connect(atonDisplay.get(), &S100DataDisplay::dataItemClicked,
            this, &MainWindow::jumpToCoordinates);

    vtsDialogInstance = new VTSDialog(this, m_ui->notificationBox);
    vtsDialogInstance->setAttribute(Qt::WA_DeleteOnClose, false); // Ensure it's not deleted on close

    connect(m_ui->notificationBox->ui->notificationList, &QListWidget::itemClicked, this, &MainWindow::showVTSDialog);

    scanATONFolder();
    scanNWFolder();

    vtsDialogInstance->pullNewVTSMessagesInit();
    vtsDialogInstance->scanAndLoad();
    vtsDialogInstance->drawNewMessages();

    deactivateVTS();

    // getSummaryDisplay = std::make_unique<getSummaryDialog>();

    MSRMenuModule = std::make_unique<MSRMainModule>();
    connect(MSRMenuModule.get(), &MSRMainModule::serviceChange, this, &MainWindow::onServiceChange);
    MSRMenuModule->init();
}

void MainWindow::initializeShipSystems()
{
    // shipDataHandler = new ShipDataHandler();
    // shipDataHandler->initializeShip();

    // try
    // {
    //     const Config &config = ConfigManager::getInstance().getConfig();

    //     nmeaProcessor = new NmeaProcessor(m_ui->conningBox, shipDataHandler, config.com_port.value());
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
    // setupShipOverlay();
    // ownShipOverlay->Set(shipDataHandler->ownShip);

    // connect(shipDataHandler, &ShipDataHandler::dataUpdated, this, &MainWindow::updateConning);
}

void MainWindow::initializeTimers()
{
    nwTimer = new QTimer(this);
    atonTimer = new QTimer(this);

    frameTimer = new QTimer(this);

    frameTimer->setInterval(200);
    frameTimer->start();

    // Automatic pulling off
    // nwTimer->start(11 * 60 * 1000);
    // atonTimer->start(10 * 60 * 1000);

    nwTimer->stop();
    atonTimer->stop();

    connect(atonTimer, &QTimer::timeout, this, &MainWindow::fullATONReload);
    connect(nwTimer, &QTimer::timeout, this, &MainWindow::fullNWReload);
    connect(frameTimer, &QTimer::timeout, this, &MainWindow::moveToShip);
}

void MainWindow::initializeFontMenu()
{
    // No need to save the default font now since we're not affecting global settings.

    QMenu *fontMenu = new QMenu("Font Size", this);

    QAction *defaultFont = new QAction("Default", this);
    QAction *smallFont = new QAction("Small", this);
    QAction *mediumFont = new QAction("Medium", this);
    QAction *largeFont = new QAction("Large", this);

    fontMenu->addAction(defaultFont);
    fontMenu->addAction(smallFont);
    fontMenu->addAction(mediumFont);
    fontMenu->addAction(largeFont);

    // Function to apply font to nwDialogInstance and atonDialogInstance
    auto applyFontToDialogInstances = [&](const QFont &font)
    {
        // Apply the font directly to the dialog instances
        nwDisplay->getDialog()->setFont(font);
        atonDisplay->getDialog()->setFont(font);

        nwDisplay->getDialog()->setStyleSheet(QString("font-size: %1px;").arg(font.pointSize()));
        atonDisplay->getDialog()->setStyleSheet(QString("font-size: %1px;").arg(font.pointSize()));
    };

    connect(defaultFont, &QAction::triggered, this, [=]()
            {
        // Reset the font to the default size for dialog instances
        applyFontToDialogInstances(QFont("Courier New", 22)); // Or set to your default font

        // Re-polish the app to apply the changes (though it's not necessary anymore since we're not changing global settings)
        qApp->style()->unpolish(qApp);
        qApp->style()->polish(qApp); });

    connect(smallFont, &QAction::triggered, this, [=]()
            {
        QFont font("Courier New", 14);
        applyFontToDialogInstances(font);

        qApp->style()->unpolish(qApp);
        qApp->style()->polish(qApp); });

    connect(mediumFont, &QAction::triggered, this, [=]()
            {
        QFont font("Courier New", 18);
        applyFontToDialogInstances(font);

        qApp->style()->unpolish(qApp);
        qApp->style()->polish(qApp); });

    connect(largeFont, &QAction::triggered, this, [=]()
            {
        QFont font("Courier New", 26);
        applyFontToDialogInstances(font);

        qApp->style()->unpolish(qApp);
        qApp->style()->polish(qApp); });

    mainMenuBar->addMenu(fontMenu);
    defaultFont->trigger();
}

// Method to reposition floating widgets on window resize
void MainWindow::resizeEvent(QResizeEvent *event)
{
    // QMainWindow::resizeEvent(event); // Call base class implementation
    int padding = 10;
    //     m_ui->conningBox->move(m_ui->chartWidget->width() - m_ui->conningBox->width() - padding, padding);
    //     m_ui->notificationBox->move(m_ui->chartWidget->width() - m_ui->notificationBox->width() - padding,
    //                                 m_ui->chartWidget->height() - m_ui->notificationBox->height() - padding);
}

QPixmap svgToPixmap(const QString &svgFilePath, const QSize &size)
{
    // Check if the SVG file exists
    if (!QFile::exists(svgFilePath))
    {
        qDebug() << "SVG file does not exist:" << svgFilePath;
        return QPixmap(); // Return an empty QPixmap
    }

    // Create an SVG renderer
    QSvgRenderer svgRenderer(svgFilePath);
    if (!svgRenderer.isValid())
    {
        qDebug() << "Invalid SVG file:" << svgFilePath;
        return QPixmap(); // Return an empty QPixmap
    }

    // Create a QImage with the desired size
    QImage image(size, QImage::Format_RGBA8888_Premultiplied);
    image.fill(Qt::transparent); // Fill with transparent background

    // Render the SVG onto the QImage
    QPainter painter(&image);
    svgRenderer.render(&painter);
    painter.end();

    // Convert QImage to QPixmap and return
    return QPixmap::fromImage(image);
}

void MainWindow::generateIcons()
{

    QDirIterator it("../resources", QStringList() << "*.svg", QDir::Files);
    while (it.hasNext())
    {
        qDebug() << it.next();
    }

    QSize iconSize(64, 64);
    QSize nwIconSize(48, 32);
    QSize moonSize(48, 48);

    m_ui->bottomToolbar->setIconSize(iconSize);
    m_ui->bottomToolbar->setContentsMargins(0, 0, 0, 0);
    m_ui->bottomToolbar->layout()->setSpacing(0);

    m_ui->actionDayNightMode->setIcon(svgToPixmap("../resources/moonsun.svg", moonSize));
    m_ui->actionRoutePlanning->setIcon(svgToPixmap("../resources/routeplan.svg", iconSize));
    m_ui->actionDisplayMode->setIcon(svgToPixmap("../resources/disp.svg", iconSize));
    m_ui->actionLockToShip->setIcon(svgToPixmap("../resources/center.svg", iconSize));
    m_ui->actionNorthUp->setIcon(svgToPixmap("../resources/nup.svg", nwIconSize));
    m_ui->actionNWMenu->setIcon(svgToPixmap("../resources/nw.svg", nwIconSize));
    // m_ui->actionVTSMenu->setIcon(svgToPixmap("../resources/vts.svg", nwIconSize));
    m_ui->actionOverlays->setIcon(svgToPixmap("../resources/settings.svg", iconSize));
    m_ui->actionATONMenu->setIcon(svgToPixmap("../resources/aton.svg", nwIconSize));

    QPixmap normal = svgToPixmap("../resources/vts.svg", nwIconSize);
    QPixmap disabled = normal;
    {
        QPainter p(&disabled);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(disabled.rect(), QColor(100, 100, 100)); // gray
    }
    QIcon icon;
    icon.addPixmap(normal, QIcon::Normal, QIcon::Off);
    icon.addPixmap(disabled, QIcon::Disabled, QIcon::Off);

    m_ui->actionVTSMenu->setIcon(icon);
}

// Override the close event
void MainWindow::closeEvent(QCloseEvent *event)
{
}

void MainWindow::onClick(QMouseEvent *event)
{
    viewPortLocked = false;
    //     m_ui->chartWidget->UnlockViewport();
    updateLockToShipButtonStyle(viewPortLocked);
};

// Initialization of the UI components
void MainWindow::initializeChart()
{
    // std::cout << "Setting chart widget " << '\n';

    // encProductlist = {"INT.IHO.S-101.2.0", "INT.IHO.S-101.1.1"};

    // // Set default settings
    // m_ui->chartWidget->SetDefaultChartSettings();
    // m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::OpenGLOffscreen);
    // m_ui->chartWidget->SetAsyncLoadingMode(Nautilus::Portrayal::EnChartDisplay::AsyncLoadingMode::Off);
    // m_ui->chartWidget->SetLoadingStrategy(Nautilus::Widgets::EnChartWidget::LoadingStrategy::ENC);
    // m_ui->chartWidget->SetAntialiasingMode(Nautilus::Widgets::EnChartWidget::AntialiasingMode::Fast);
    // m_ui->chartWidget->SetPreviewMode(Nautilus::Widgets::EnChartWidget::PreviewMode::Disabled);
    // m_ui->chartWidget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    // m_ui->chartWidget->SetTargetFPS(60);
    // m_ui->chartWidget->SetSymbolDeclutteringMode(true);
    // m_ui->chartWidget->SetAutoRepaintEnabled(true);
    // // m_ui->chartWidget->SetPreviewModeBorder(1.0);
    // std::cout << "Setting the ENC product display modes:" << '\n';
    // try
    // {
    //     m_ui->chartWidget->SetDisplayMode(encProductlist, Nautilus::Portrayal::EnDisplayMode::DisplayCategory::Standard);
    // }
    // catch (const Nautilus::EnException &ex)
    // {

    //     QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
    //     std::cout << "Funny happened: " << msg.toStdString() << '\n';
    //     QMessageBox::critical(nullptr, "", msg);
    // }

    // connect(m_ui->chartWidget, &Nautilus::Widgets::EnChartWidget::sigMousePress, this, &MainWindow::onClick);
    // connect(m_ui->chartWidget, &Nautilus::Widgets::EnChartWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
}

void MainWindow::changeAsyncMode()
{
    bool ok;
    QStringList loadingStrategies = {"Immediate", "Wait", "Off"};
    QString loadingChoice = QInputDialog::getItem(this, "Select Async Load", "Async Strategy:", loadingStrategies, 0, false, &ok);
    if (ok && !loadingChoice.isEmpty())
    {
        if (loadingChoice == "Off")
        {
            //             m_ui->chartWidget->SetAsyncLoadingMode(Nautilus::Portrayal::EnChartDisplay::AsyncLoadingMode::Off);
        }
        else if (loadingChoice == "Wait")
        {
            //             m_ui->chartWidget->SetAsyncLoadingMode(Nautilus::Portrayal::EnChartDisplay::AsyncLoadingMode::WaitDraw);
        }
        else if (loadingChoice == "Immediate")
        {
            //             m_ui->chartWidget->SetAsyncLoadingMode(Nautilus::Portrayal::EnChartDisplay::AsyncLoadingMode::ImmediateDraw);
        }
    }
}

// Slot to change painter mode
void MainWindow::changePainterMode()
{
    bool ok;
    QStringList painterModes = {"None", "Cairo", "Qt", "VulkanOffscreen", "VulkanSurface", "OpenGLOffscreen", "OpenGLSurface", "OpenGLSplit", "OpenGLSplitTextures"};
    QString painterChoice = QInputDialog::getItem(this, "Select Painter Mode", "Painter Mode:", painterModes, 0, false, &ok);
    if (ok && !painterChoice.isEmpty())
    {
        if (painterChoice == "None")
        {
            //             m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::None);
        }
        else if (painterChoice == "Cairo")
        {
            //             m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::Cairo);
        }
        else if (painterChoice == "Qt")
        {
            //             m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::Qt);
        }
        else if (painterChoice == "VulkanOffscreen")
        {
            //             m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::VulkanOffscreen);
        }
        else if (painterChoice == "VulkanSurface")
        {
            //             m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::VulkanSurface);
        }
        else if (painterChoice == "OpenGLOffscreen")
        {
            //             m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::OpenGLOffscreen);
        }
        else if (painterChoice == "OpenGLSurface")
        {
            //             m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::OpenGLSurface);
        }
        else if (painterChoice == "OpenGLSplit")
        {
            //             m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::OpenGLSplit);
        }
        else if (painterChoice == "OpenGLSplitTextures")
        {
            //             m_ui->chartWidget->SetPainterMode(Nautilus::Widgets::EnChartWidget::PainterMode::OpenGLSplitTextures);
        }
    }
}

// Slot to change loading strategy
void MainWindow::changeLoadingStrategy()
{
    bool ok;
    QStringList loadingStrategies = {"None", "ENC", "BathymetricENC", "AML", "Custom"};
    QString loadingChoice = QInputDialog::getItem(this, "Select Loading Strategy", "Loading Strategy:", loadingStrategies, 0, false, &ok);
    if (ok && !loadingChoice.isEmpty())
    {
        if (loadingChoice == "None")
        {
            //             m_ui->chartWidget->SetLoadingStrategy(Nautilus::Widgets::EnChartWidget::LoadingStrategy::None);
        }
        else if (loadingChoice == "ENC")
        {
            //             m_ui->chartWidget->SetLoadingStrategy(Nautilus::Widgets::EnChartWidget::LoadingStrategy::ENC);
        }
        else if (loadingChoice == "BathymetricENC")
        {
            //            m_ui->chartWidget->SetLoadingStrategy(Nautilus::Widgets::EnChartWidget::LoadingStrategy::BathymetricENC);
        }
        else if (loadingChoice == "AML")
        {
            //             m_ui->chartWidget->SetLoadingStrategy(Nautilus::Widgets::EnChartWidget::LoadingStrategy::AML);
        }
        else if (loadingChoice == "Custom")
        {
            //             m_ui->chartWidget->SetLoadingStrategy(Nautilus::Widgets::EnChartWidget::LoadingStrategy::Custom);
        }
    }
}

// Slot to change antialiasing mode
void MainWindow::changeAntialiasingMode()
{
    bool ok;
    QStringList antialiasingModes = {"None", "Fast", "Best"};
    QString antialiasingChoice = QInputDialog::getItem(this, "Select Antialiasing Mode", "Antialiasing Mode:", antialiasingModes, 0, false, &ok);
    if (ok && !antialiasingChoice.isEmpty())
    {
        if (antialiasingChoice == "None")
        {
            //             m_ui->chartWidget->SetAntialiasingMode(Nautilus::Widgets::EnChartWidget::AntialiasingMode::None);
        }
        else if (antialiasingChoice == "Fast")
        {
            //             m_ui->chartWidget->SetAntialiasingMode(Nautilus::Widgets::EnChartWidget::AntialiasingMode::Fast);
        }
        else if (antialiasingChoice == "Best")
        {
            //             m_ui->chartWidget->SetAntialiasingMode(Nautilus::Widgets::EnChartWidget::AntialiasingMode::Best);
        }
    }
}

// Slot to change preview mode
void MainWindow::changePreviewMode()
{
    bool ok;
    QStringList previewModes = {"Disabled", "Static", "Dynamic"};
    QString previewChoice = QInputDialog::getItem(this, "Select Preview Mode", "Preview Mode:", previewModes, 0, false, &ok);
    if (ok && !previewChoice.isEmpty())
    {
        if (previewChoice == "Disabled")
        {
            //             m_ui->chartWidget->SetPreviewMode(Nautilus::Widgets::EnChartWidget::PreviewMode::Disabled);
        }
        else if (previewChoice == "Static")
        {
            //             m_ui->chartWidget->SetPreviewMode(Nautilus::Widgets::EnChartWidget::PreviewMode::Static);
        }
        else if (previewChoice == "Dynamic")
        {
            //             m_ui->chartWidget->SetPreviewMode(Nautilus::Widgets::EnChartWidget::PreviewMode::Dynamic);
        }
    }
}

void MainWindow::setupShipOverlay()
{
    //     ownShipOverlay = m_ui->chartWidget->CreateOverlay<Nautilus::OwnShip::EnOwnShipOverlay>();
    // ownShipOverlay->SetLayer(2U);

    // ownShipOverlay->SetVelocityVectorDuration(600.0f, 30.0f);
    // // ownShipOverlay->SetPastTrackDuration(1.0f, 1.0f);
    // // ownShipOverlay->SetGuardZone(Nautilus::Monitoring::EnGuardZone(Nautilus::EN_PI_4, 5U));

    // using namespace Nautilus::OwnShip;
    // auto appearance = ownShipOverlay->GetAppearance();

    // appearance |= EnOwnShipOverlay::Appearance::GuardZone;

    // // appearance |= EnOwnShipOverlay::Appearance::VelocityVector;

    // // appearance |= EnOwnShipOverlay::Appearance::VelocityVectorWithTimeIncrements;

    // appearance |= EnOwnShipOverlay::Appearance::PrimaryPastTrack;

    // appearance |= EnOwnShipOverlay::Appearance::PrimaryPastTrackWithTimeIncrements;

    // appearance |= EnOwnShipOverlay::Appearance::PrimaryPastTrackWithPastPositions;

    // appearance |= EnOwnShipOverlay::Appearance::PrimaryPastTrackWithLabels;

    // // appearance |= EnOwnShipOverlay::Appearance::SecondaryPastTrack;

    // // appearance |= EnOwnShipOverlay::Appearance::SecondaryPastTrackWithTimeIncrements;

    // // appearance |= EnOwnShipOverlay::Appearance::SecondaryPastTrackWithPastPositions;

    // // appearance |= EnOwnShipOverlay::Appearance::SecondaryPastTrackWithLabels;

    // ownShipOverlay->SetAppearance(appearance);
}

void MainWindow::changeColourPalette()
{
    // Get the current palette
    // Nautilus::Portrayal::EnPalette currentPalette = m_ui->chartWidget->GetPalette();
    // Nautilus::Portrayal::EnPalette newPalette = currentPalette;

    // Create the context menu
    QMenu dayNightMenu(this);

    // Create actions for Day, Dusk, and Night modes
    QAction dayAction("Day Mode", this);
    QAction duskAction("Dusk Mode", this);
    QAction nightAction("Night Mode", this);

    // Connect actions to set newPalette only
    // connect(&dayAction, &QAction::triggered, this, [&newPalette]()
    //         { newPalette = Nautilus::Portrayal::EnPalette::Day; });

    // connect(&duskAction, &QAction::triggered, this, [&newPalette]()
    //         { newPalette = Nautilus::Portrayal::EnPalette::Dusk; });

    // connect(&nightAction, &QAction::triggered, this, [&newPalette]()
    //         { newPalette = Nautilus::Portrayal::EnPalette::Night; });

    // Add actions to the menu
    dayNightMenu.addAction(&dayAction);
    dayNightMenu.addAction(&duskAction);
    dayNightMenu.addAction(&nightAction);

    QPoint mousePos = QCursor::pos();                      // Get the global position of the mouse
    QAction *selectedAction = dayNightMenu.exec(mousePos); // Show the menu at the mouse position
    if (selectedAction)
    {
        //         m_ui->chartWidget->SetPalette(newPalette);
        //         m_ui->chartWidget->ForceUpdate();
    }
}

void MainWindow::showDisplayCategoryMenu()
{
    QMenu displayCategoryMenu(this);

    // Create actions for each DisplayCategory
    QAction displayBaseAction("Display Base Layer", this);
    QAction standardAction("Display Standard Layer", this);
    QAction otherAction("Display All Layers", this);
    QAction noCategoryAction("No Category", this);

    // Connect each action to changeViewGroup with the appropriate DisplayCategory
    // connect(&displayBaseAction, &QAction::triggered, this, [this]()
    //         { changeViewGroup(Nautilus::Portrayal::EnDisplayMode::DisplayCategory::DisplayBase); });

    // connect(&standardAction, &QAction::triggered, this, [this]()
    //         { changeViewGroup(Nautilus::Portrayal::EnDisplayMode::DisplayCategory::Standard); });

    // connect(&otherAction, &QAction::triggered, this, [this]()
    //         { changeViewGroup(Nautilus::Portrayal::EnDisplayMode::DisplayCategory::Other); });

    // connect(&noCategoryAction, &QAction::triggered, this, [this]()
    //         { changeViewGroup(Nautilus::Portrayal::EnDisplayMode::DisplayCategory::NoCategory); });

    // Add actions to the menu
    displayCategoryMenu.addAction(&displayBaseAction);
    displayCategoryMenu.addAction(&standardAction);
    // displayCategoryMenu.addAction(&otherAction);
    // displayCategoryMenu.addAction(&noCategoryAction);

    // Show the menu at the mouse position
    displayCategoryMenu.exec(QCursor::pos());
}

// void MainWindow::changeViewGroup(Nautilus::Portrayal::EnDisplayMode::DisplayCategory category)
// {
//     // Try to set the display mode, handling any exceptions
//     try
//     {
//         //         m_ui->chartWidget->SetDisplayMode(encProductlist, category);
//         //         m_ui->chartWidget->ForceUpdate();
//     }
//     catch (const Nautilus::EnException &ex)
//     {
//         QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
//         QMessageBox::critical(nullptr, "", msg);
//         std::cerr << "Nautilus error at view group" << msg.toStdString() << "\n";
//     }
//     catch (const std::exception &ex)
//     {
//         std::cerr << "An unexpected error occurred: " << ex.what() << "\n";
//     }
// }

void MainWindow::loadChart()
{
    // try
    // {
    //     auto cellPath = Nautilus::Widgets::EnChartWidget::ShowLoadCellDialog();
    //     auto filename = Nautilus::EnString(cellPath.toStdWString());
    //     addDataSetToChartWidget(filename, 100);
    // }
    // catch (const Nautilus::EnException &ex)
    // {
    //     QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
    //     QMessageBox::critical(nullptr, "", msg);
    // }
}

// std::vector<Nautilus::EnString> MainWindow::getFilesInFolder(std::string folderPath)
// {

//     std::vector<Nautilus::EnString> filelist;
//     namespace fs = std::filesystem;
//     try
//     {
//         fs::path folder(folderPath);

//         // Check if the folder exists and is a directory, exit early if not
//         if (!fs::exists(folder))
//         {
//             std::cerr << "Folder does not exist: " << folder.string() << "\n";
//         }
//         if (!fs::is_directory(folder))
//         {
//             std::cerr << "Path is not a directory: " << folder.string() << "\n";
//         }

//         // Iterate over files in the folder
//         for (const auto &entry : fs::directory_iterator(folder))
//         {
//             if (fs::is_regular_file(entry.path()))
//             {
//                 std::string enstringed = entry.path().string();
//                 Nautilus::EnString ensttr = Nautilus::EnString(enstringed);
//                 filelist.push_back(ensttr);

//                 std::cout << ensttr << '\n';
//             }
//         }
//     }
//     catch (const std::filesystem::filesystem_error &ex)
//     {
//         std::cerr << "Filesystem error: " << ex.what() << "\n";
//     }
//     catch (const Nautilus::EnException &ex)
//     {
//         QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
//         QMessageBox::critical(nullptr, "Error", msg);
//     }
//     catch (const std::exception &ex)
//     {
//         std::cerr << "An unexpected error occurred: " << ex.what() << "\n";
//     }
//     return filelist;
// };

void MainWindow::loadFullChartSet()
{
    loadGeneralCharts(100);
    loadApproaches(200);
    loadTurkuHarbor(300);
}

void MainWindow::loadGeneralCharts(int layer)
{
    // auto filelist = getFilesInFolder(ConfigManager::getInstance().getConfig().general_charts.value());
    // for (auto file : filelist)
    // {
    //     addDataSetToChartWidget(file, layer);
    // }
}

void MainWindow::loadDanishCharts(int layer)
{
    // auto filelist = getFilesInFolder(ConfigManager::getInstance().getConfig().danish_charts.value());
    // for (auto file : filelist)
    // {
    //     addDataSetToChartWidget(file, layer);
    // }
}

void MainWindow::loadApproaches(int layer)
{
    // if (layer < 2)
    // {
    //     layer = 200;
    // }
    // auto filelist = getFilesInFolder(ConfigManager::getInstance().getConfig().all_approaches.value());
    // for (auto file : filelist)
    // {
    //     addDataSetToChartWidget(file, layer);
    // }
}
void MainWindow::loadTurkuHarbor(int layer)
{
    // auto filelist = getFilesInFolder(ConfigManager::getInstance().getConfig().turku_harbor.value());
    // for (auto file : filelist)
    // {
    //     addDataSetToChartWidget(file, layer);
    // }
}

void MainWindow::clearCharts()
{
    // try
    // {
    //         m_ui->chartWidget->Clear();
    // }
    // catch (const Nautilus::EnException &ex)
    // {
    //     QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
    //     QMessageBox::critical(nullptr, "", msg);
    // }
}

// void MainWindow::addDataSetToChartWidget(Nautilus::EnString dataPath, int layer)
// {

//     std::cout << "Loading dataset..." << '\n';
//     try
//     {
//         const QString qpath = QString::fromStdString(dataPath.ToStdString());
//         auto dataset = Nautilus::DataAccess::EnDataSet::Load(dataPath, Nautilus::DataAccess::EnDataSet::Key(), false, true);

//         m_ui->chartWidget->AddDataSet(dataset, layer);
//         m_ui->chartWidget->CenterOnLoadedDatasets();

//         std::unique_ptr<RockFilter> funFilter = std::make_unique<RockFilter>();
//         m_ui->chartWidget->GetChartDisplay()->SetFeatureFilter(std::move(funFilter));
//     }
//     catch (const Nautilus::EnException &ex)
//     {
//         QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
//         QMessageBox::critical(nullptr, "", msg);
//     }
// };

void MainWindow::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    QPointF mousePos = pos;

    QAction action1("Pick Report", this);
    connect(&action1, &QAction::triggered, this, [this, pos]()
            { MainWindow::onOption1(pos); });
    contextMenu.addAction(&action1);

    // QAction action2("Create route / add waypoint", this);
    // connect(&action2, &QAction::triggered, this, [this, pos]()
    //         { MainWindow::onOption2(pos); });
    // contextMenu.addAction(&action2);

    // QAction action3("Set Origin Point", this);
    // connect(&action3, &QAction::triggered, this, [this, pos]()
    //         { MainWindow::onOption3(pos); });
    // contextMenu.addAction(&action3);

    // QAction action4("Set Target Point", this);
    // connect(&action4, &QAction::triggered, this, [this, pos]()
    //         { MainWindow::onOption4(pos); });
    // contextMenu.addAction(&action4);

    // QAction action5("Set Origin to Ship", this);
    // connect(&action5, &QAction::triggered, this, &MainWindow::onOption5);
    // contextMenu.addAction(&action5);

    contextMenu.exec(mapToGlobal(pos));
}
// inline QString ToQString(const Nautilus::EnString &string)
// {
//     return QString::fromStdWString(string.ToStdWString());
// }

// inline QTreeWidgetItem *createItemFromPickReport(QTreeWidgetItem *&parent, const std::shared_ptr<Nautilus::PickReport::EnPickReportFeature> &pickFeature)
// {
//     QTreeWidgetItem *featureItem = new QTreeWidgetItem(parent);

//     Nautilus::EnString itemName = pickFeature->GetName();
//     featureItem->setText(0, QString::fromStdWString(itemName.ToStdWString()));
//     QString gstr = ToQString(pickFeature->GetGeometricPrimitiveText());
//     featureItem->setText(2, QString("Feature Type (%1)").arg(gstr));

//     const auto &attributes = pickFeature->GetAttributes();

//     for (const auto &attribute : attributes)
//     {
//         auto attributeName = attribute->GetName();
//         if (attributeName == "Scale Minimum")
//         {
//             continue;
//         }
//         QTreeWidgetItem *attributeItem = new QTreeWidgetItem(featureItem);
//         QString attributeType = QString("Attribute (%1)").arg(ToQString(attribute->GetTypeName()));
//         // std::cout << "Valuecount of attribute " << attribute->GetName() << " " << attribute->GetValueCount() << '\n';

//         for (int i = 0; i < attribute->GetValueCount(); i++)
//         {
//             std::cout << attribute->GetValue(i) << '\n';
//         }
//         if (attributeName == "Feature Name")
//         {
//             auto subAttris = attribute->GetAttributes();
//             // Make a check here to only have the longer version
//             for (auto subatt : subAttris)
//             {
//                 attributeItem->setText(0, ToQString(subatt->GetName()));
//                 attributeItem->setText(1, ToQString(subatt->GetValue(0U)));
//                 attributeItem->setText(2, attributeType);
//             }
//         }
//         else
//         {
//             attributeItem->setText(0, ToQString(attribute->GetName()));
//             attributeItem->setText(1, ToQString(attribute->GetValue(0U)));
//             attributeItem->setText(2, attributeType);
//         }
//     }
//     return featureItem;
// }

void MainWindow::onOption1(const QPoint &pos)
{
    //     QList<QTreeWidgetItem *> items;

    //     Nautilus::Geometry::EnPoint pnt(pos.x(), pos.y());
    //         auto report = m_ui->chartWidget->PickReport(pnt, 12U);
    //         auto dynreport = navwarnOverlay->Pick(m_ui->chartWidget->GetViewPort(), pnt, 12U);
    //         auto atonDynReport = atonOverlay->Pick(m_ui->chartWidget->GetViewPort(), pnt, 12U);
    //     if (report == nullptr && dynreport.Count() == 0 && atonDynReport.Count() == 0)
    //     {
    //         qDebug() << "No reports found";
    //         return;
    //     }
    //     m_ui->chartWidget->ShowPickReportDialog(*report.get());
    //     QDialog *dialog = new QDialog(this);
    //     QTreeWidget *treeWidget = new QTreeWidget(dialog);
    //     QStringList headers = {"Name", "Value"};
    //     treeWidget->setColumnCount(2);
    //     treeWidget->setHeaderLabels(headers);

    //     // Process dynamic reports if they exist
    //     if (dynreport.Count() > 0)
    //     {
    //         auto &dynFeatures = dynreport.GetEntries();
    //         for (auto entry : dynFeatures)
    //         {
    //             qDebug() << "Looping dyn report";
    //             auto *pickResult = static_cast<NAVWARNDynamicOverlay::NAVWARNPickInfo *>(entry.GetItem()->GetPickInfo());
    //             if (pickResult && pickResult->navwarnQTreeInfo)
    //             {
    //                 QTreeWidgetItem *featureItem = pickResult->navwarnQTreeInfo->clone();
    //                 qDebug() << featureItem;
    //                 qDebug() << featureItem->child(0);
    //                 items.append(featureItem);
    //                 qDebug() << "Adding dyn report";
    //             }
    //         }
    //     }

    //     // Process dynamic reports if they exist
    //     if (atonDynReport.Count() > 0)
    //     {
    //         auto &dynFeatures = atonDynReport.GetEntries();
    //         for (auto entry : dynFeatures)
    //         {
    //             qDebug() << "Looping dyn report";
    //             auto *pickResult = static_cast<ATONDynamicOverlay::ATONPickInfo *>(entry.GetItem()->GetPickInfo());
    //             if (pickResult && pickResult->atonQTreeInfo)
    //             {
    //                 QTreeWidgetItem *featureItem = pickResult->atonQTreeInfo->clone();
    //                 qDebug() << featureItem;
    //                 qDebug() << featureItem->child(0);
    //                 items.append(featureItem);
    //                 qDebug() << "Adding dyn report";
    //             }
    //         }
    //     }

    //     // Process standard report if it exists
    //     QTreeWidgetItem *pickReportItem = nullptr;
    //     if (report != nullptr)
    //     {
    //         Nautilus::EnString str = report->GetPositionText();
    //         QString reportText = QString("Pick Report at : %1").arg(QString::fromStdWString(str.ToStdWString()));

    //         pickReportItem = new QTreeWidgetItem();
    //         pickReportItem->setText(0, reportText);

    //         const auto &features = report->GetFeatures();
    //         for (const auto &feature : features)
    //         {
    //             Nautilus::EnString itemName = feature->GetName();
    //             auto stringName = itemName.ToStdString();
    //             QString qname = QString::fromStdString(stringName);
    //             qDebug() << qname;

    //             if (stringName == "User Chart Object")
    //             {
    //                 qDebug() << "Found UC";
    //             }
    //             else
    //             {
    //                 QTreeWidgetItem *featureItem = createItemFromPickReport(pickReportItem, feature);
    //                 items.append(featureItem);
    //             }
    //         }

    //         treeWidget->addTopLevelItem(pickReportItem);
    //     }

    //     // Insert all collected items
    //     treeWidget->insertTopLevelItems(0, items);
    //     treeWidget->expandAll();
    //     treeWidget->resizeColumnToContents(0);

    //     // Set up the layout for the dialog
    //     QVBoxLayout *layout = new QVBoxLayout(dialog);
    //     layout->addWidget(treeWidget);
    //     dialog->setLayout(layout);
    //     dialog->setWindowTitle("Pick Report");
    //     dialog->resize(600, 400);

    //     // Show the dialog
    //     dialog->exec();

    //     // Cleanup
    //     delete dialog;
}

void MainWindow::pointNorth()
{
    //     m_ui->chartWidget->ResetRotation();
}

// void MainWindow::onOption2(const QPoint &pos)
// {
//     Nautilus::Geometry::EnPoint pnt(pos.x(), pos.y());
//     Nautilus::Geodesy::EnGeoPoint geoPoint = m_ui->chartWidget->GetViewPort().Reverse(pnt);

//     Nautilus::Route::EnWaypoints waypoints = route->GetWaypoints();
//     if (!waypoints.HasDefaultWaypoint())
//     {
//         Nautilus::Route::EnDefaultWaypoint waypoint;

//         // Modify the leg settings
//         auto leg = waypoint.GetLeg();
//         leg.SetStarboardXTD(0.05f);
//         leg.SetPortsideXTD(0.05f);
//         leg.SetSafetyDepth(10.0f);
//         leg.SetMasthead(12.0f);

//         // Modify some waypoint settings
//         waypoint.SetRadius(0.0f);
//         waypoint.SetLeg(leg);

//         // Add to the waypoints reference of the route.
//         waypoints.SetDefaultWaypoint(waypoint);
//     }
//     auto wpIt = waypoints.Add(geoPoint);
//     route->SetWaypoints(waypoints);
//     routeOverlay->SetRoute(route);
// }

// void MainWindow::onOption3(const QPoint &pos)
// {
//     Nautilus::Geometry::EnPoint pnt(pos.x(), pos.y());
//     Nautilus::Geodesy::EnGeoPoint geopos = m_ui->chartWidget->DeviceToGeo(pnt);
//     navTools->tempErblOrigin->SetPosition(geopos);
// }

// void MainWindow::onOption4(const QPoint &pos)
// {
//     Nautilus::Geometry::EnPoint pnt(pos.x(), pos.y());
//     Nautilus::Geodesy::EnGeoPoint geopos = m_ui->chartWidget->DeviceToGeo(pnt);
//     navTools->tempErblSecondPoint->SetPosition(geopos);

//     if (navTools->getInstances().size() == 0)
//     {
//         navTools->createInstance();
//     }
// }

// void MainWindow::onOption5()
// {
//     originAtShip = true;
// }

// void MainWindow::runRoute()
// {
//     auto waypoints = route->GetWaypoints();
//     if (waypoints.Count() > 1 && !shipIsSetupped)
//     {
//         setupSimulator();
//         shipIsSetupped = true;
//         m_ui->chartWidget->ForceUpdate();
//         shipSimulator->startSimulation();
//     }
//     else
//     {
//         shipSimulator->stopSimulation();
//     }
// }

void MainWindow::updateLockToShipButtonStyle(bool locked)
{
    QWidget *widget = m_ui->bottomToolbar->widgetForAction(m_ui->actionLockToShip);
    QToolButton *button = qobject_cast<QToolButton *>(widget);

    m_ui->actionLockToShip->setChecked(locked);
    if (button)
    {
        if (locked)
            button->setStyleSheet("border: 2px solid yellow; border-radius: 4px;");
        else
            button->setStyleSheet("");
    }
}

void MainWindow::lockToShip()
{
    if (viewPortLocked)
    {
        viewPortLocked = false;
    }
    else
    {
        //         m_ui->chartWidget->MoveTo(shipDataHandler->ownShip->GetPosition().GetValue());
        viewPortLocked = true;
    }

    updateLockToShipButtonStyle(viewPortLocked);
}

void MainWindow::moveToShip()
{
    if (viewPortLocked)
    {
        //         m_ui->chartWidget->MoveTo(shipDataHandler->ownShip->GetPosition().GetValue());
    }
}

void MainWindow::applyScale(double min, double max)
{
    // try
    // {
    //     //         m_ui->chartWidget->SetScaleRange(min, max);
    // }
    // catch (const Nautilus::EnException &ex)
    // {
    //     QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
    //     QMessageBox::critical(nullptr, "", msg);
    // }
}

void MainWindow::showNWDialog()
{
    // Show the dialog
    nwDisplay->getDialog()->show();
}

void MainWindow::showVTSDialog()
{
    // Show the dialog
    vtsDialogInstance->updateDefaultTime();
    vtsDialogInstance->exec();
}

void MainWindow::showATONDialog()
{
    atonDisplay->getDialog()->show();
}

void MainWindow::showMSRMenu()
{
    MSRMenuModule->showDialog();
}

void MainWindow::showSummary()
{
    getSummaryDisplay->exec();
}

void MainWindow::clearRelatedNotification(VTSDigitalInformationMessage *message)
{
    // Show the dialog
    QListWidget *notificationList = m_ui->notificationBox->findChild<QListWidget *>("notificationList");
    if (notificationList)
    {
        qDebug() << "Removing";

        int i = 0;
        while (i < notificationList->count())
        {
            QListWidgetItem *item = notificationList->item(i);
            qDebug() << QVariant::fromValue(reinterpret_cast<quintptr>(message));

            if (item->data(Qt::UserRole) == QVariant::fromValue(reinterpret_cast<quintptr>(message)))
            {
                // Remove the item from the widget
                notificationList->takeItem(i);
                // Delete the item to free memory
                delete item;
                qDebug() << "Item removed.";
                // Do NOT increment `i` because the list is now shorter
            }
            else
            {
                ++i; // Only increment if no item was removed
            }
        }
    }
}

void MainWindow::jumpToCoordinates(DirectPosition *position)
{
    //     m_ui->chartWidget->MoveTo(position->getNautilusPoint());
}

inline QString convertToNS(double latitude)
{
    QString direction = latitude >= 0 ? "N" : "S";
    latitude = std::fabs(latitude);

    int degrees = static_cast<int>(latitude);
    double fractional = latitude - degrees;

    int minutes = static_cast<int>(fractional * 60);
    double seconds = (fractional * 60 - minutes) * 60;

    return QString("%1°%2'%3\" %4")
        .arg(degrees)
        .arg(minutes)
        .arg(seconds, 0, 'f', 2)
        .arg(direction);
}

inline QString convertToWE(double longitude)
{
    QString direction = longitude >= 0 ? "E" : "W";
    longitude = std::fabs(longitude);

    int degrees = static_cast<int>(longitude);
    double fractional = longitude - degrees;

    int minutes = static_cast<int>(fractional * 60);
    double seconds = (fractional * 60 - minutes) * 60;

    return QString("%1°%2'%3\" %4")
        .arg(degrees)
        .arg(minutes)
        .arg(seconds, 0, 'f', 2)
        .arg(direction);
}

void MainWindow::updateConning(double lat, double lon, double sog, double head)
{
    qDebug() << "Received AIS data: Latitude =" << lat
             << ", Longitude =" << lon
             << ", Speed =" << sog
             << ", Heading =" << head;

    QString coords = convertToNS(lat) + '\n' + convertToWE(lon);

    QLabel *label2 = m_ui->conningBox->findChild<QLabel *>("label_2");
    QLabel *label3 = m_ui->conningBox->findChild<QLabel *>("label_3");
    QLabel *label4 = m_ui->conningBox->findChild<QLabel *>("label_4");
    QLabel *speedLabel = m_ui->conningBox->findChild<QLabel *>("speedLabel");
    QLabel *headingLabel = m_ui->conningBox->findChild<QLabel *>("headingLabel");
    QLabel *positionLabel = m_ui->conningBox->findChild<QLabel *>("positionLabel");

    m_ui->conningBox->updatePosition(lat, lon);
    // Set the text for the labels
    if (speedLabel)
    {
        speedLabel->setText(QString::number(sog) + " knots"); // Convert speed to string
    }

    if (headingLabel)
    {
        headingLabel->setText(QString::number(head) + "°"); // Convert heading to string
    }

    if (positionLabel)
    {
        // positionLabel->setText(coords); // Set the coordinates (NS/WE) in position label
    }
}
void MainWindow::scanNWFolder()
{
    std::vector<S100FeatureWrapper *> undrawnList = nwDisplay->scanAndDraw();
    for (auto undrawn : undrawnList)
    {
        // navwarnOverlay->drawNAVWARN(static_cast<NAVWARNWrapper *>(undrawn));
    }
}
void MainWindow::scanATONFolder()
{
    std::vector<S100FeatureWrapper *> undrawnList = atonDisplay->scanAndDraw();
    for (auto undrawn : undrawnList)
    {
        // atonOverlay->drawATON(static_cast<ATONWrapper *>(undrawn));
    }
}

void MainWindow::fullATONReload()
{

    // Check that atons actually get updated, do not clear before
    // try
    // {
    //     //         atonOverlay = m_ui->chartWidget->CreateOverlay<ATONDynamicOverlay>();
    //     // atonOverlay->SetLayer(1000);
    // }
    // catch (const Nautilus::EnException &ex)
    // {
    //     QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
    //     QMessageBox::critical(nullptr, "", msg);
    // }

    auto undrawnList = atonDisplay->fullReload();
    // for (auto undrawn : undrawnList)
    // {
    //     atonOverlay->drawATON(static_cast<ATONWrapper *>(undrawn));
    // }
};

void MainWindow::fullNWReload()
{
    //     try
    //     {
    //         //         navwarnOverlay = m_ui->chartWidget->CreateOverlay<NAVWARNDynamicOverlay>();
    //         navwarnOverlay->SetLayer(1000);
    //     }
    //     catch (const Nautilus::EnException &ex)
    //     {
    //         QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
    //         QMessageBox::critical(nullptr, "", msg);
    //         return;
    //     }

    auto undrawnList = nwDisplay->fullReload();
    //     std::cout << "Drawing navwarns " << '\n';
    //     for (auto *undrawn : undrawnList)
    //     {
    //         try
    //         {
    //             navwarnOverlay->drawNAVWARN(static_cast<NAVWARNWrapper *>(undrawn));
    //         }
    //         catch (const Nautilus::EnException &ex)
    //         {
    //             QString msg = QString::fromStdWString(ex.GetInfo().ToStdWString());
    //             QMessageBox::critical(nullptr, "", msg);
    //         }
    //         catch (...)
    //         {
    //             std::cerr << "Something happened when drawing navwarn" << '\n';
    //         }
    //     }
    //     std::cout << "All navwarns drawn " << '\n';
}

void MainWindow::onServiceChange(const S100Service &service)
{
    std::cout << "updating service " << service.name.toStdString() << '\n';
    std::cout << "Service active in MainWindow?: " << service.active << '\n';

    if (service.dataProductType == "S124")
    {
        if (service.active)
        {
            nwDisplay->getDataLayer()->addService(service);
        }
        else
        {
            nwDisplay->getDataLayer()->removeService(service);
        }
        return;
    }

    if (service.dataProductType == "S125")
    {

        if (service.active)
        {
            atonDisplay->getDataLayer()->addService(service);

            // getSummaryDisplay->defaultService = service;
            // getSummaryDisplay->updateUI();
        }
        else
        {
            atonDisplay->getDataLayer()->removeService(service);
        }
        return;
    }

    if (service.dataProductType == "S212")
    {

        if (service.active)
        {
            vtsDialogInstance->setService(service);
            auto activated = vtsDialogInstance->activateService();
            vtsDialogInstance->startVTSLoop();
            m_ui->actionVTSMenu->setEnabled(true);
            m_ui->actionVTSMenu->setToolTip("Opens the VTS dialog");
        }
        else
        {
            deactivateVTS();
        }
    }
}

void MainWindow::deactivateVTS()
{
    vtsDialogInstance->stopVTSLoop();
    vtsDialogInstance->clearService();
    vtsDialogInstance->deactivateService();
    m_ui->actionVTSMenu->setEnabled(false);
    m_ui->actionVTSMenu->setToolTip("No VTS service active. Check MSR menu.");
}
#include "moc_MainWindow.cpp"
