#pragma once

/*!
 * \file vtsdialog.h
 * \brief Provides the dialog for displaying and interacting with VTS messages and threads.
 *
 * Handles a message-based interaction system between vessels and a VTS authority.
 * Supports:
 * - Tabbed message threads
 * - Message composition, sending, acknowledgment
 * - Server sync with async updates
 * - Notification integration
 * - Thread-aware UI
 */

#include <QDialog>
#include <QListWidgetItem>
#include <QToolBar>
#include <QMap>
#include <QMessageBox>
#include <QMenu>
#include <QProgressDialog>
#include <QButtonGroup>
#include <QApplication>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

#include <map>
#include <vector>
#include <string>

#include "VTSMessageWrapper.hpp"
#include "VTSDigitalInformationMessage.hpp"
#include "VTSDataLayer.hpp"
#include "Resources.hpp"
#include "floatingnotificationbox.h"
#include "ui_floatingnotificationbox.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include "S100Service.hpp"
namespace Ui
{
    class VTSDialog;
}

/*!
 * \class VTSDialog
 * \brief Dialog interface for viewing, creating, and managing VTS message threads.
 *
 * This widget provides the main interface for interaction with the VTS clearance system.
 * Features:
 * - Manages a set of message threads, each shown in a tab
 * - Composes messages with subject, location, and time
 * - Periodically syncs with a remote server
 * - Manages acknowledgment states of messages
 * - Integrates with floating UI notifications
 */
class VTSDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor.
     * \param parent Parent widget.
     * \param notificationBox Notification UI component for alerts and status messages.
     */
    explicit VTSDialog(QWidget *parent = nullptr, FloatingNotificationBox *notificationBox = nullptr);
    ~VTSDialog();
    /*!
     * \brief Starts acknowledgment thread and enters event loop.
     * \return Result of QDialog::exec().
     */
    int exec() override;

    /*!
     * \brief Adds a new message to a specific thread tab.
     * \param message Pointer to the message to add.
     * \param threadID Numeric ID representing the thread.
     */
    void addMessageToThread(VTSMessageWrapper *message, int threadID);

    /*!
     * \brief Sets the own-ship identifier used in outgoing messages.
     * \param vesselID String containing the vessel ID.
     */
    void loadVesselID(const std::string &vesselID);

    /*!
     * \brief Forces a reload of locations from the configuration file.
     */
    void rePopulateLocations();

    /*!
     * \brief Adds all unseen messages from cache into UI.
     */
    void drawNewMessages();

    /*!
     * \brief Scans and parses raw VTS messages from disk.
     */
    void scanAndLoad();

    /*!
     * \brief Updates message widgets to reflect latest acknowledgment statuses.
     */
    void updateACKStatus();

    /*!
     * \brief Replaces current ACK list with a new one.
     * \param newMap Mapping from message UUID to ACK type value.
     * \return True if map was changed, false if identical.
     */
    bool loadAckMap(std::map<std::string, int> newMap);

    /*!
     * \brief Resets the effective time input field to now.
     */
    void updateDefaultTime();

    /*!
     * \brief Begins an asynchronous message pull from the VTS server.
     */
    void pullNewVTSMessages();

    /*!
     * \brief Performs a blocking message pull and updates the UI.
     */
    void pullNewVTSMessagesInit();

    /*!
     * \brief Starts a periodic update loop (default 2.5 seconds).
     */
    void startVTSLoop();

    /*!
     * \brief Stops the background sync timer.
     */
    void stopVTSLoop();

    /*!
     * \brief Returns a reference to the set of UUIDs that have been rendered in the UI.
     */
    QSet<QString> &getDisplayedMessages();

    /*!
     * \brief Maps thread IDs to their corresponding message list widgets.
     */
    QMap<int, QListWidget *> threadListWidgets;

    /*!
     * \brief Sets which VTS endpoint is used.
     */
    void setService(const S100Service &service);

    /*!
     * \brief Removes the set service
     * \return Returns false if no service was set
     */
    bool clearService();

    /*!
     * \brief Activates the present service
     * \return Returns false if no service was set
     */
    bool activateService();

    /*!
     * \brief Dectivates the present service
     * \return Returns false if no service was set
     */
    bool deactivateService();

signals:
    /*!
     * \brief Emitted when a message has been successfully sent.
     */
    void messageSent();

    /*!
     * \brief Emitted when a notification related to a message should be cleared.
     * \param message Pointer to the message to clear.
     */
    void clearNotification(VTSDigitalInformationMessage *message);

private slots:
    /*!
     * \brief Handles "Send" button click and message dispatch.
     */
    void on_newMessageButton_clicked();

    /*!
     * \brief Creates a new thread tab and initializes data.
     */
    void on_newThreadButton_clicked();

    /*!
     * \brief Opens context menu on a message tab for closing.
     * \param pos Cursor position where menu is invoked.
     */
    void showThreadContextMenu(const QPoint &pos);

    /*!
     * \brief Handles list selection and manages ACK when user selects a message.
     * \param item Pointer to the selected QListWidgetItem.
     */
    void selectionChanged(QListWidgetItem *item);

    /*!
     * \brief Called when the user switches tabs; triggers ACK scan and item hook-up.
     * \param index Index of the newly activated tab.
     */
    void tabChange(int index);

private:
    // UI and structure
    Ui::VTSDialog *ui;                        ///< Pointer to UI form.
    FloatingNotificationBox *notificationBox; ///< External notification UI component.
    std::optional<S100Service> vtsService;

    // Thread control
    QToolBar *toolBar = nullptr;
    QAction *acknowledgeAction = nullptr;
    QAction *sendNewMessageAction = nullptr;
    QButtonGroup *toFromButtonGroup = nullptr;

    // Message layer and identifiers
    std::unique_ptr<VTSDataLayer> vtsDataLayer;
    std::string ownVesselID;
    std::string subscriptionID;

    // State and memory tracking
    QListWidgetItem *currentSelection = nullptr;
    QSet<QString> displayedMessages;
    QMap<QListWidgetItem *, VTSMessageWidget *> itemWidgetMap;

    // Location and subject handling
    std::map<std::string, std::string> displayToUrnMap;
    std::map<std::string, std::string> urnToDisplayMap;

    // Acknowledgment cache
    std::map<std::string, int> ackList;

    // Timing control
    std::vector<int> timeOffsets;
    QTimer *vtsTimer = nullptr;
    bool newMessageLock = false;

    // Internal helpers
    void setupTime();
    void populateSubjectList();
    void populateStatusList();
    void populateLocationList();
    void updateFloatingNotificationBox();
    void setSendingAvailability(bool availability);
    void setTopicAvailability(bool availability);
    void lockMessageCheck();
    void acknowledgeThread(int threadNumber);
    void acknowledgeOnExec();
    void updateAckStatuses();
    void updatePendingAckVisuals();
    void loadVTSMessages();
    void closeThread();
    VTSMessageWrapper *writeNewMessageToFile();
};