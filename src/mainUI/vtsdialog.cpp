#include "vtsdialog.h"
#include "ui_vtsdialog.h"
#include <QDebug>

VTSDialog::VTSDialog(QWidget *parent, FloatingNotificationBox *passedNotificationBox)
    : QDialog(parent), ui(new Ui::VTSDialog), notificationBox(passedNotificationBox)
{
    ui->setupUi(this);

    ownVesselID = ConfigManager::getInstance().getConfig().vessel_id.value();

    // vtsDataLayer = new VTSDataLayer("../data/VTS", ownVesselID);
    vtsDataLayer = std::make_unique<VTSDataLayer>("../data/VTS", ownVesselID);
    // Buttons already connected in the UI file
    ui->messageThreads->removeTab(1);
    ui->messageThreads->removeTab(0);

    ui->messageThreads->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->messageThreads, &QTabWidget::customContextMenuRequested, this, &VTSDialog::showThreadContextMenu);
    connect(ui->sendButton, &QPushButton::clicked, this, &VTSDialog::on_newMessageButton_clicked);
    connect(ui->closeThreadButton, &QPushButton::clicked, this, &VTSDialog::closeThread);

    connect(ui->messageThreads, &QTabWidget::currentChanged, this, &VTSDialog::tabChange);

    QString defaultStyle = "QPushButton { background-color: #333; color: white; border: 2px solid #888; padding: 5px; }";
    QString selectedStyle = "QPushButton { background-color: #555; color: white; border: 2px solid #FFF; padding: 5px; }";

    ui->messageThreads->setStyleSheet(R"(
        QListWidget::item:selected {
            background: transparent;
            border: 2px solid white;
            color: white;
        }
        QListWidget::item:selected:active {
            background: transparent;
            border: 2px solid white;
            color: white;
        }
        QTabBar::tab { height: 80px; }
    )");

    // Set initial styles
    ui->toButton->setStyleSheet(defaultStyle);
    ui->fromButton->setStyleSheet(defaultStyle);

    toFromButtonGroup = new QButtonGroup(this);
    toFromButtonGroup->addButton(ui->toButton, 0);
    toFromButtonGroup->addButton(ui->fromButton, 1);
    // Connect button clicks to toggle styles
    connect(toFromButtonGroup, &QButtonGroup::idClicked, this, [=](int id)
            {
                if(id == 0 ) {
                    ui->toButton->setStyleSheet(selectedStyle);
                    ui->toButton->setChecked(true);
                    ui->fromButton->setStyleSheet(defaultStyle);
                    ui->fromButton->setChecked(false);
                } else if (id == 1){
                    ui->fromButton->setStyleSheet(selectedStyle);
                    ui->fromButton->setChecked(true);
                    ui->toButton->setStyleSheet(defaultStyle);
                    ui->toButton->setChecked(false);
                } });

    populateSubjectList();
    populateStatusList();
    populateLocationList();
    setupTime();

    // Connect combo box selection change to button group update
    connect(ui->vtsSubjects, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [=](int index)
            {
        QString selectedText = ui->vtsSubjects->itemText(index);
        if (selectedText.contains("Arrival", Qt::CaseInsensitive)) {
            toFromButtonGroup->button(0)->click();  // Simulate "To" button click
        } else if (selectedText.contains("Departure", Qt::CaseInsensitive)) {
            toFromButtonGroup->button(1)->click();  // Simulate "From" button click
        } });
    toFromButtonGroup->button(0)->click();
    setSendingAvailability(false);
    loadVTSMessages();
    // Polling starts when an S-212 service is activated (MainWindow::onServiceChange)
}

VTSDialog::~VTSDialog()
{
    stopVTSLoop();
    delete ui;
}

int VTSDialog::exec()
{
    std::thread([this]()
                {
                    std::cout << "Thread started for acknowledgeOnExec.\n";
                    acknowledgeOnExec(); // Run acknowledgeOnExec in the background
                })
        .detach(); // Detach the thread to run independently

    lockMessageCheck();
    return QDialog::exec(); // Call the original exec()
}

void VTSDialog::lockMessageCheck()
{

    int currentTabIndex = ui->messageThreads->currentIndex();
    if (currentTabIndex < 0)
    {
        return;
    }
    int threadNumber = ui->messageThreads->property("tab" + currentTabIndex).toInt();
    qDebug() << "Thread number: " << threadNumber;

    QListWidget *listWidget = qobject_cast<QListWidget *>(ui->messageThreads->widget(currentTabIndex));

    setSendingAvailability(true);
    auto activeThreads = vtsDataLayer->getActiveThreads();
    auto it = std::find(activeThreads.begin(), activeThreads.end(), threadNumber);

    if (it != activeThreads.end())
    {
        setSendingAvailability(false);
        setTopicAvailability(true);
        QListWidgetItem *item = listWidget->item(0);
        if (!item)
        {
            std::cout << "huh<>" << '\n';
            std::cout << "Trying to create a thread but nothing exists" << '\n';
            return;
        }

        // Retrieve the wrapper from the item
        VTSMessageWrapper *messageWrapper = reinterpret_cast<VTSMessageWrapper *>(item->data(Qt::UserRole).value<quintptr>());
        if (!messageWrapper)
        {
            std::cout << "huh<??>" << '\n';
        }

        // Find the correct subject and select it in the combo box
        QString subject = QString::fromStdString(messageWrapper->getSubject());
        int subjectIndex = ui->vtsSubjects->findText(subject);
        if (subjectIndex != -1)
        {
            ui->toButton->setEnabled(true);
            ui->fromButton->setEnabled(true);
            ui->vtsSubjects->setCurrentIndex(subjectIndex);

            QString selectedText = ui->vtsSubjects->itemText(currentTabIndex);

            ui->toButton->setEnabled(false);
            ui->fromButton->setEnabled(false);
        }
        else
        {
            qDebug() << "Subject not found in combo box: " << subject;
        }

        // Find the correct location and select it in the combo box
        QString locationURN = QString::fromStdString(messageWrapper->getLocation());
        auto itLocation = std::find_if(displayToUrnMap.begin(), displayToUrnMap.end(),
                                       [&locationURN](const auto &pair)
                                       { return pair.second == locationURN.toStdString(); });

        if (itLocation != displayToUrnMap.end())
        {
            int locationIndex = ui->vtsLocations->findText(QString::fromStdString(itLocation->first));
            if (locationIndex != -1)
            {
                ui->vtsLocations->setCurrentIndex(locationIndex);
            }
        }
        else
        {
            qDebug() << "Location URN not found in combo box: " + locationURN;
        }
    }
};

void VTSDialog::showThreadContextMenu(const QPoint &pos)
{
    int tabIndex = ui->messageThreads->tabBar()->tabAt(pos);
    if (tabIndex == -1)
        return; // Click wasn't on a tab

    QMenu contextMenu;
    QAction *unsubscribeAction = contextMenu.addAction("Close");

    QAction *selectedAction = contextMenu.exec(ui->messageThreads->mapToGlobal(pos));
    if (selectedAction == unsubscribeAction)
    {
        int threadNumber = ui->messageThreads->property("tab" + tabIndex).toInt();

        vtsDataLayer->unsubscribeThread(threadNumber);
        std::cout << "Closing thread " << threadNumber << '\n';
        vtsDataLayer->closeThread(threadNumber);
        ui->messageThreads->removeTab(tabIndex);
    }
}

void VTSDialog::closeThread()
{
    int tabIndex = ui->messageThreads->currentIndex();
    if (tabIndex == -1)
        return; // No conversations

    int threadNumber = ui->messageThreads->property("tab" + tabIndex).toInt();

    QString tabTitle = ui->messageThreads->tabText(tabIndex);
    if (tabTitle == "NEW TRAFFIC CLEARANCE")
    {
        ui->messageThreads->removeTab(tabIndex);
        if (ui->messageThreads->count() == 0)
        {
            ui->newThreadButton->setEnabled(true);
        }
        return;
    };

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Close Thread",
        QString("Make sure that the clearance process has been fully completed before closing. Are you sure you want to close the discussion '%1'?").arg(tabTitle),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;
    std::cout << "Closing thread " << threadNumber << '\n';

    vtsDataLayer->unsubscribeThread(threadNumber);
    vtsDataLayer->closeThread(threadNumber);
    ui->messageThreads->removeTab(tabIndex);
    currentSelection = nullptr;
}

VTSMessageWrapper *VTSDialog::writeNewMessageToFile()
{

    qDebug() << "New message creation initiated";

    auto *newMessage = new VTSDigitalInformationMessage();
    auto *newClearance = new ClearanceMessage();
    qDebug() << "message and clearance initiated";

    // Step 1: Determine the reference message (if replying in an existing thread)
    int currentIndex = ui->messageThreads->currentIndex();
    std::optional<std::string> referenceMessageUUID;
    qDebug() << "current index " << currentIndex;

    newMessage->vesselID = ownVesselID;
    newMessage->reportedBy = ownVesselID;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1000, 9999);
    std::string portCallIDSuffix = std::to_string(dis(gen));

    newMessage->portCallID = ownVesselID + ":" + portCallIDSuffix;
    int threadNumber = ui->messageThreads->property("tab" + currentIndex).toInt();

    std::optional<std::string> latestMessageUUID;
    qDebug() << "current thread " << threadNumber;

    try
    {
        latestMessageUUID = vtsDataLayer->getLatestMessageInThread(threadNumber);
        // std::cout << "latest message is : " << latestMessageUUID.value() << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database error while fetching latest message: " << e.what() << std::endl;
    }

    // Assign reference ID from the latest message in the thread
    if (latestMessageUUID.has_value() && latestMessageUUID != "")
    {
        referenceMessageUUID = latestMessageUUID.value();
        newMessage->referenceMessageID = referenceMessageUUID;

        auto wrapper = vtsDataLayer->getMessageCache().find(referenceMessageUUID.value())->second;
        qDebug() << QString::fromStdString(wrapper->getPortCallID());
        if (!wrapper->getPortCallID().empty())
        {
            newMessage->portCallID = wrapper->getPortCallID();
        }
        newClearance->effectiveTime = wrapper->getEffectiveTime();
        newClearance->windowAfter = wrapper->getWindowAfter();
    }
    else
    {
        int timeOffSetMinutes = timeOffsets[ui->vtsTimeWindow->currentIndex()];
        auto qDateTime = ui->vtsEffectiveTime->dateTime();

        // Convert to string format (ISO 8601)
        QString dateTimeString = qDateTime.toString(Qt::ISODate);

        newClearance->effectiveTime = DateTime(dateTimeString.toStdString());

        int hours = timeOffSetMinutes / 60;
        int minutes = timeOffSetMinutes % 60;
        QString timeOffsetString = QString("%1:%2")
                                       .arg(hours, 2, 10, QChar('0'))    // Ensure two-digit hour
                                       .arg(minutes, 2, 10, QChar('0')); // Ensure two-digit minute

        newClearance->windowAfter = timeOffsetString.toStdString();
    }

    qDebug() << "Assigning subjects etc ";

    // Step 2: Assign subject and status
    int subjectIndex = ui->vtsSubjects->currentIndex();
    int statusIndex = ui->vtsStatus->currentIndex();

    QString selectedText = ui->vtsLocations->currentText();
    std::string location = displayToUrnMap[selectedText.toStdString()];

    newClearance->subjectOfCommunication = new SubjectOfCommunication(subjectIndex);
    newClearance->messageStatus = new MessageStatus(statusIndex);

    auto *newLocation = new VTSLocation();

    bool toIsCheck = ui->toButton->isChecked();
    if (toIsCheck)
    {
        newLocation->locationType = new LocationType(1);
    }
    else
    {
        newLocation->locationType = new LocationType(0);
    }
    newLocation->locationURN = location;

    newClearance->vtsLocations.push_back(newLocation);
    newMessage->clearancePart.push_back(newClearance);

    newMessage->reportedAt = DateTime();
    newMessage->comment = ui->vtsCommentBox->toPlainText().toStdString();

    qDebug() << "Wrapping message";
    auto *wrapper = new VTSMessageWrapper(newMessage, Sender::Ship);
    wrapper->generateUUID();
    wrapper->threadNumber = threadNumber;
    std::cout << "Thread number is here : " << threadNumber << '\n';
    try
    {
        wrapper->writeToFile();
    }
    catch (const std::exception &e)
    {
        // The message is invalid or could not be written; do not store or send it
        std::cerr << "Error in writing: " << e.what() << std::endl;
        throw;
    }
    // Step 3: Determine thread number
    try
    {
        VTSMessageRepository::DBMessageRow message;

        if (referenceMessageUUID.has_value())
        {
            threadNumber = vtsDataLayer->getThreadNumberOfRefID(referenceMessageUUID.value());
            std::cout << "Thread number is from db : " << threadNumber << '\n';
            message.refUUID = referenceMessageUUID.value();
        }
        message.uuid = wrapper->uuid;
        message.threadNumber = threadNumber;
        message.sender = "Ship";
        message.filename = wrapper->filename;
        vtsDataLayer->insertMessage(message);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database error in message creation: " << e.what() << std::endl;
    }
    return wrapper;
}

void VTSDialog::on_newMessageButton_clicked()
{

    QProgressDialog progress("Sending message...", nullptr, 0, 0);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setCancelButton(nullptr);
    progress.show();
    QApplication::processEvents();

    VTSMessageWrapper *wrapper = nullptr;
    try
    {
        wrapper = writeNewMessageToFile();
    }
    catch (const std::exception &e)
    {
        progress.close();
        QMessageBox::critical(this, "Message not sent", QString::fromStdString(e.what()));
        return;
    }
    // Run in another thread

    bool somethingHappened = false;
    QString errorMessage;

    QFuture<void> future = QtConcurrent::run([this, wrapper, &somethingHappened, &errorMessage]
                                             {

    // Step 4: Handle subscription (if starting a new thread)
    std::string subscriptionID;
    auto referenceMessageUUID = wrapper->getReferenceId();
    auto threadNumber = wrapper->threadNumber;
    if (referenceMessageUUID.empty())
    {
        qDebug() << "Starting a new thread with uuid " + QString::fromStdString(wrapper->uuid);

        try
        {
            vtsDataLayer->subscribe(wrapper->uuid, threadNumber);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in subscription request: " << e.what() << std::endl;
            somethingHappened = true;
            errorMessage += "Error during subscription:\n" + QString::fromStdString(e.what());
        }
    }

    try
    {
        vtsDataLayer->postMessage(wrapper->uuid);
        acknowledgeThread(threadNumber);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Something went wrong in posting: " << e.what() << '\n';
        somethingHappened = true;
        errorMessage += "Error during message posting:\n" + QString::fromStdString(e.what());

    }

    // QListWidget *listWidget = qobject_cast<QListWidget *>(ui->messageThreads->widget(threadNumber));
    // connect(listWidget, &QListWidget::itemClicked, this, &VTSDialog::selectionChanged);
    qDebug() << "Message Sending complete"; });

    while (!future.isFinished())
    {
        QApplication::processEvents();
    }
    progress.close();

    if (somethingHappened)
    {
        QMessageBox::critical(this, "Error", errorMessage);
        return;
    }

    // emit messageSent();
    try
    {
        // Step 6: Add message to UI
        qDebug() << "adding message to thread";
        int currentIndex = ui->messageThreads->currentIndex();

        addMessageToThread(wrapper, wrapper->threadNumber);
        qDebug() << "scan loop now ";
        auto currentTitle = ui->messageThreads->tabText(currentIndex);
        if (currentTitle == "NEW TRAFFIC CLEARANCE")
        {
            // QString tabTitle = QString::fromStdString(wrapper->getSubject());
            std::string tabTitle = wrapper->getSubject() + '\n' + urnToDisplayMap[wrapper->getLocation()];
            ui->messageThreads->setTabText(currentIndex, QString::fromStdString(tabTitle));

        }
        scanAndLoad();
        ui->newThreadButton->setEnabled(true);
        setSendingAvailability(false);
        setTopicAvailability(true);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Something funny at the end of new message: " << e.what() << std::endl;
    }
}

void VTSDialog::acknowledgeOnExec()
{
    // Get the current active tab index
    int currentTabIndex = ui->messageThreads->currentIndex();

    if (currentTabIndex >= 0)
    {
        int threadNumber = ui->messageThreads->property("tab" + currentTabIndex).toInt();
        acknowledgeThread(threadNumber);
    }
}

void VTSDialog::selectionChanged(QListWidgetItem *item)
{
    std::cout << "Connecting selection change" << '\n';
    qDebug() << "Selection changes " << '\n';
    qDebug() << item;
    currentSelection = item;
    ;

    if (!currentSelection)
        return;

    VTSMessageWrapper *wrapper = reinterpret_cast<VTSMessageWrapper *>(currentSelection->data(Qt::UserRole).value<quintptr>());
    auto uuid = wrapper->uuid;

    // Check if acknowledgment is needed
    if (vtsDataLayer->needsAcknowledgment(uuid))
    {
        try
        {
            vtsDataLayer->acknowledgeMessage(uuid, 2);
            vtsDataLayer->updateAckForMessage(uuid);
        }

        catch (const std::exception &e)
        {
            std::cerr << "Ack error when clicking: " << e.what() << std::endl;
        }
    }
    else
    {
        qDebug() << "Message already acknowledged: " << QString::fromStdString(uuid);
    }
}

void VTSDialog::acknowledgeThread(int threadNumber)
{

    std::vector<std::string> pendingAcks = vtsDataLayer->getPendingAcksForThread(threadNumber);
    // Only proceed if there are messages needing acknowledgment
    if (!pendingAcks.empty())
    {
        for (const auto &uuid : pendingAcks)
        {
            vtsDataLayer->acknowledgeMessage(uuid, 2);
        }

        // Mark them as acknowledged in the database
        vtsDataLayer->assignAckToThread(threadNumber);
    }
    else
    {
        qDebug() << "No pending acknowledgments for this thread.";
    }
}

void VTSDialog::tabChange(int index)
{
    qDebug() << "Tab changed to index:" << index;

    int threadNumber = ui->messageThreads->property("tab" + index).toInt();
    qDebug() << "Thread number: " << threadNumber;

    acknowledgeThread(threadNumber);

    QListWidget *listWidget = qobject_cast<QListWidget *>(ui->messageThreads->widget(index));
    if (listWidget)
    {
        std::cout << "Connecting selection change" << '\n';
        connect(listWidget, &QListWidget::itemClicked, this, &VTSDialog::selectionChanged);
    }
    if (index >= 0)
    {
        lockMessageCheck();
    }
}

void VTSDialog::setService(const S100Service &service)
{
    vtsService = service;
}

bool VTSDialog::clearService()
{
    if (vtsService)
    {
        vtsService.reset();
        return true;
    }
    return false;
}

bool VTSDialog::activateService()
{
    if (vtsService)
    {
        vtsService->active = true;
        vtsDataLayer->setEndpoint(vtsService->endpoint);
        return true;
    }
    return false;
}

bool VTSDialog::deactivateService()
{
    if (vtsService)
    {
        vtsService->active = false;
        vtsDataLayer->clearEndpoint();

        return true;
    }
    return false;
}

void VTSDialog::setSendingAvailability(bool availability)
{
    ui->sendButton->setEnabled(availability);
    ui->vtsCommentBox->setEnabled(availability);
    ui->vtsEffectiveTime->setEnabled(availability);
    ui->vtsLocations->setEnabled(availability);
    ui->vtsSubjects->setEnabled(availability);
    ui->vtsStatus->setEnabled(availability);
    ui->vtsTimeWindow->setEnabled(availability);
    ui->toButton->setEnabled(availability);
    ui->fromButton->setEnabled(availability);
}

void VTSDialog::setTopicAvailability(bool availability)
{
    ui->sendButton->setEnabled(availability);
    ui->vtsCommentBox->setEnabled(availability);
    // ui->vtsEffectiveTime->setEnabled(availability);
    ui->vtsStatus->setEnabled(availability);
    // ui->vtsTimeWindow->setEnabled(availability);
}

void VTSDialog::updateACKStatus()
{
    for (auto it = threadListWidgets.begin(); it != threadListWidgets.end(); ++it)
    {
        QListWidget *listWidget = it.value();
        if (!listWidget)
            continue;

        for (int i = 0; i < listWidget->count(); ++i)
        {
            QListWidgetItem *item = listWidget->item(i);
            if (!item)
                continue;

            // Retrieve the wrapper from the item
            VTSMessageWrapper *messageWrapper = reinterpret_cast<VTSMessageWrapper *>(item->data(Qt::UserRole).value<quintptr>());
            if (!messageWrapper)
                continue;

            std::string uuid = messageWrapper->uuid; // Assuming this function exists in VTSMessageWrapper
            std::cout << "Looking for " << uuid << '\n';
            // Check if the message exists in ackList
            auto ackIt = ackList.find(uuid);

            if (ackIt != ackList.end())
            {
                std::cout << "iterator not empty for " << uuid << '\n';

                int ackTypeValue = ackIt->second;
                // messageWrapper->acknowledged = (static_cast<ackType>(ackTypeValue)); // Update acknowledgment status
                std::cout << "found ack: " << uuid << " " << ackTypeValue << '\n';
                // Set item color based on ackType enum

                // Retrieve the associated widget
                VTSMessageWidget *vtsMessageWidget = itemWidgetMap.value(item, nullptr); // Retrieve the widget
                if (vtsMessageWidget)
                {
                    // If the widget is found, update its status
                    std::cout << "Updating ack status " << uuid << '\n';
                    vtsMessageWidget->updateStatus(ackTypeValue);
                }

                switch (static_cast<ackType>(ackTypeValue))
                {
                case Delivered: // Message successfully delivered
                    // item->setBackground(Qt::yellow); // Yellow indicates "Delivered"
                    item->setToolTip("Message has been delivered to the receiver's side.");

                    break;
                case Opened: // Message opened by the recipient
                    // item->setBackground(Qt::green); // Green indicates "Opened"
                    item->setToolTip("Message has been opened on the receiver's side.");
                    break;
                case Error: // There was an error in delivery
                    // item->setBackground(Qt::red); // Red indicates "Error"
                    item->setToolTip("Error in ACK");

                    break;
                default:
                    // item->setBackground(Qt::white); // Default color for unknown states
                    break;
                }
            }
        }
    }
}

bool VTSDialog::loadAckMap(std::map<std::string, int> newMap)
{
    // Compare if the maps are the same
    if (ackList == newMap)
    {
        // If the maps are identical, return false (no update needed)
        return false;
    }

    // Otherwise, update the map and return true (update performed)
    ackList = newMap;
    // vtsDataLayer->updateAckTypes(ackList);
    return true;
}

void VTSDialog::on_newThreadButton_clicked()
{
    // Generate a unique thread ID by looking through the threadListWidgets map
    int threadID = 1;

    try
    {
        threadID = vtsDataLayer->highestThreadNumber() + 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database error while fetching highest thread number: " << e.what() << std::endl;
        QMessageBox::critical(this, "Database Error", "An error occurred while fetching the highest thread number.");
        return; // Exit early if the database query fails
    }

    // Create the new list widget for the thread
    QListWidget *listWidget = new QListWidget();
    listWidget->setObjectName("messageList");
    // Store the list widget in the map with the new thread ID
    QString tabTitle = QString("NEW TRAFFIC CLEARANCE");

    int index = ui->messageThreads->addTab(listWidget, tabTitle);
    threadListWidgets[threadID] = listWidget;

    // Update the tab title to reflect the new thread's ID
    std::cout << "Creating new thread " << threadID;
    // QString tabTitle = QString("NEW THREAD %1").arg(threadID);

    // Add the tab for the new thread
    // ui->messageThreads->setUserData(threadID, QVariant::fromValue(threadID));

    // ui->messageThreads->setUserData(index, new TabIdentifierData(threadID));
    ui->messageThreads->setProperty("tab" + index, QVariant(threadID));

    ui->newThreadButton->setEnabled(false);
}

void VTSDialog::loadVesselID(const std::string &vesselID)
{
    ownVesselID = vesselID;
}

void VTSDialog::populateSubjectList()
{
    for (int i = 0; i < 11; ++i)
    {
        SubjectOfCommunication subject(i);
        ui->vtsSubjects->addItem(QString::fromStdString(subject.getSubjectOfCommunicationLiteral()));
    }
}

void VTSDialog::populateStatusList()
{
    for (int i = 0; i < 5; ++i)
    {
        MessageStatus status(i);
        ui->vtsStatus->addItem(QString::fromStdString((status.getMessageStatusLiteral())));
    }
}

void VTSDialog::rePopulateLocations()
{
    // ui->vtsLocations->clear();
    populateLocationList();
}

void VTSDialog::populateLocationList()
{
    ui->vtsLocations->clear(); // Clear previous items
    displayToUrnMap.clear();   // Clear old mappings

    if (!ConfigManager::getInstance().getConfig().location_map)
        return;

    for (const auto &[city, berths] : *ConfigManager::getInstance().getConfig().location_map)
    {
        for (const std::string &urn : berths)
        {
            std::string lastField = urn.substr(urn.find_last_of(':') + 1);
            std::string displayName = city + " " + lastField;

            ui->vtsLocations->addItem(QString::fromStdString(displayName));
            displayToUrnMap[displayName] = urn; // Store mapping
            urnToDisplayMap[urn] = displayName;
        }
    }

    int maxWidth = 0;
    QFontMetrics fm(ui->vtsLocations->font());

    for (int i = 0; i < ui->vtsLocations->count(); ++i)
    {
        int itemWidth = fm.horizontalAdvance(ui->vtsLocations->itemText(i));
        maxWidth = std::max(maxWidth, itemWidth);
    }

    // Set minimum width with some padding
    ui->vtsLocations->setMinimumWidth(maxWidth + 40);
}

void VTSDialog::setupTime()
{
    // Set the current date and time
    ui->vtsEffectiveTime->setDateTime(QDateTime::currentDateTime());

    // Vector of time offsets in minutes
    timeOffsets = {0, 5, 10, 20};

    // Clear the combo box before adding new entries
    ui->vtsTimeWindow->clear();

    // Loop over the vector and add formatted time increments to the combo box
    for (int offset : timeOffsets)
    {

        QString displayText = QString("+%1 min").arg(offset);

        // Store the actual QDateTime as user data
        ui->vtsTimeWindow->addItem(displayText);
    }
}

void VTSDialog::updateDefaultTime()
{
    ui->vtsEffectiveTime->setDateTime(QDateTime::currentDateTime());
}

void VTSDialog::addMessageToThread(VTSMessageWrapper *message, int threadID)
{
    if (!message)
        return;

    // Ensure the message isn't already displayed
    QString uuid = QString::fromStdString(message->uuid);
    if (displayedMessages.contains(uuid))
        return; // Avoid duplicates

    // Retrieve or create the correct tab for this thread ID
    QListWidget *listWidget = nullptr;

    if (threadListWidgets.contains(threadID))
    {
        listWidget = threadListWidgets[threadID];
    }
    else
    {
        // Create a new thread tab if it does not exist
        std::string tabTitle = message->getSubject() + '\n' + urnToDisplayMap[message->getLocation()];

        listWidget = new QListWidget();
        listWidget->setObjectName("messageList");

        threadListWidgets[threadID] = listWidget;

        int tabIndex = ui->messageThreads->addTab(listWidget, QString::fromStdString(tabTitle)); // Use thread ID as tab label
        // QFont testfont;
        // testfont.setPointSize(1);
        // ui->messageThreads->setFont(testfont);

        ui->messageThreads->setProperty("tab" + tabIndex, QVariant(threadID));
    }

    // Create the message item
    QListWidgetItem *messageItem = message->getQListWidgetItem();
    messageItem->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(message)));

    // Find correct insert position (newest messages first)
    int insertIndex = 0;
    for (int i = 0; i < listWidget->count(); ++i)
    {
        QListWidgetItem *existingItem = listWidget->item(i);
        VTSMessageWrapper *existingWrapper = reinterpret_cast<VTSMessageWrapper *>(existingItem->data(Qt::UserRole).value<quintptr>());

        if (existingWrapper && existingWrapper->getReportTime() > message->getReportTime())
        {
            insertIndex = i + 1; // Insert after older messages
        }
        else
        {
            break; // Stop as soon as we find a newer message
        }
    }

    // Insert message into the UI
    listWidget->insertItem(insertIndex, messageItem);
    listWidget->setItemWidget(messageItem, message->vtsMessage);

    // When you set the item widget:
    itemWidgetMap[messageItem] = message->vtsMessage;

    // Mark message as displayed
    displayedMessages.insert(uuid);
}

QSet<QString> &VTSDialog::getDisplayedMessages() { return displayedMessages; }

void VTSDialog::scanAndLoad()
{

    vtsDataLayer->scanAndParseMessages();
    // vtsDataLayer->loadMessagesFromDatabase();
};

void VTSDialog::drawNewMessages()
{
    // Get active thread numbers via data layer
    std::vector<int> activeThreadVec = vtsDataLayer->getActiveThreads();
    std::unordered_set<int> activeThreads(activeThreadVec.begin(), activeThreadVec.end());

    const std::unordered_map<std::string, VTSMessageWrapper *> &messageCache = vtsDataLayer->getMessageCache();

    for (const auto &[uuidStr, message] : messageCache)
    {
        if (!message)
            continue;

        QString uuid = QString::fromStdString(uuidStr);

        if (displayedMessages.contains(uuid))
            continue;

        int threadID = vtsDataLayer->getThreadNumberOfRefID(uuidStr);

        if (threadID < 0 || activeThreads.find(threadID) == activeThreads.end())
            continue;
        std::cout << "Adding to thread " << threadID << '\n';
        addMessageToThread(message, threadID);
        displayedMessages.insert(uuid);
    }

    updateACKStatus();
}

void VTSDialog::loadVTSMessages()
{
    qDebug() << "Initializing VTS messages...";

    scanAndLoad();
    drawNewMessages();
    // vtsDataLayer->assignAckToNewMessages();
    updateAckStatuses();
    updateFloatingNotificationBox();

    qDebug() << "VTS messages successfully initialized.";
}

void VTSDialog::updateFloatingNotificationBox()
{
    // Step 1: Check for active threads
    std::vector<int> activeThreads = vtsDataLayer->getActiveThreads(); // Get active threads from the DB

    // Step 2: Handle main item ("VTS connection active") visibility
    QListWidgetItem *mainItem = nullptr;
    for (int i = 0; i < notificationBox->ui->notificationList->count(); ++i)
    {
        QListWidgetItem *item = notificationBox->ui->notificationList->item(i);
        if (item->text() == "VTS connection active")
        {
            mainItem = item;
            break;
        }
    }

    // If no active threads, remove the main item if it exists
    if (activeThreads.empty())
    {
        if (mainItem)
        {
            delete mainItem;
        }
    }
    else
    {
        // If there are active threads, make sure the main item is present
        if (!mainItem)
        {
            mainItem = new QListWidgetItem("VTS connection active");
            notificationBox->ui->notificationList->addItem(mainItem);
        }
    }

    // Step 3: Check all threads for messages needing acknowledgment
    bool hasNewMessages = false;

    // Check each thread for pending acknowledgments
    for (int threadNumber : activeThreads)
    {
        std::vector<std::string> pendingAcks = vtsDataLayer->getPendingAcksForThread(threadNumber);

        if (!pendingAcks.empty())
        {
            hasNewMessages = true;
            break; // No need to check further if there are already pending acknowledgments
        }
    }

    // Step 4: Handle the subitem for new messages
    QListWidgetItem *subItem = nullptr;
    for (int i = 0; i < notificationBox->ui->notificationList->count(); ++i)
    {
        QListWidgetItem *item = notificationBox->ui->notificationList->item(i);
        if (item->text() == "New VTS response!" && item->foreground() == Qt::yellow)
        {
            subItem = item;
            break;
        }
    }

    // Add subitem for new messages if not already present
    if (hasNewMessages && !subItem)
    {
        subItem = new QListWidgetItem("New VTS response!");
        subItem->setForeground(Qt::yellow);
        notificationBox->ui->notificationList->addItem(subItem);
    }
    else if (!hasNewMessages && subItem)
    {
        // Remove the subitem if no new messages
        delete subItem;
    }
}

void VTSDialog::pullNewVTSMessages()
{
    try
    {
        QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>(this);

        connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]()
                {
            bool newFiles = watcher->future().result();
            watcher->deleteLater();
            //std::cout << "In the pull loop " << '\n';
            if (newFiles)
            {
                scanAndLoad();
                drawNewMessages();
                QApplication::beep();
                vtsDataLayer->assignAckToNewMessages();
                qDebug() << "VTS messages successfully updated.";
            }
            //std::cout << "updating acks" << '\n';
            updateAckStatuses();
            updatePendingAckVisuals();
            updateFloatingNotificationBox(); });

        QFuture<bool> future = QtConcurrent::run([this]()
                                                 { return vtsDataLayer->syncWithServer(); });

        watcher->setFuture(future);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error at VTS message pulling: " << e.what() << std::endl;
    }
}

void VTSDialog::pullNewVTSMessagesInit()
{
    try
    {
        bool newFiles = vtsDataLayer->syncWithServer();
        std::cout << "In the initial pull loop " << '\n';
        if (newFiles)
        {
            scanAndLoad();
            drawNewMessages();
            QApplication::beep();
            vtsDataLayer->assignAckToNewMessages();
            qDebug() << "VTS messages successfully updated.";
        }
        std::cout << "updating acks" << '\n';
        updateAckStatuses();
        updatePendingAckVisuals();
        updateFloatingNotificationBox();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error at VTS message pulling: " << e.what() << std::endl;
    }
}

void VTSDialog::updatePendingAckVisuals()
{
    // Loop over all thread numbers in the threadListWidgets map
    for (auto it = threadListWidgets.begin(); it != threadListWidgets.end(); ++it)
    {
        QListWidget *listWidget = it.value();
        if (!listWidget)
            continue;

        // Get the list of pending acknowledgments for this thread
        int threadNumber = it.key();
        std::vector<std::string> pendingAcks = vtsDataLayer->getPendingAcksForThread(threadNumber);
        std::unordered_set<std::string> pendingSet(pendingAcks.begin(), pendingAcks.end());

        // Loop over each item in the QListWidget and update the indicator
        for (int i = 0; i < listWidget->count(); ++i)
        {
            QListWidgetItem *item = listWidget->item(i);
            if (!item)
                continue;

            // Retrieve the message wrapper from the item
            VTSMessageWrapper *messageWrapper = reinterpret_cast<VTSMessageWrapper *>(item->data(Qt::UserRole).value<quintptr>());
            if (!messageWrapper)
                continue;

            // Get the UUID of the current message
            std::string uuid = messageWrapper->uuid;

            // Find the corresponding widget for this item
            VTSMessageWidget *widget = itemWidgetMap.value(item, nullptr);
            if (!widget)
                continue;

            // Check if the message UUID is in the pending acknowledgment list
            bool isNew = pendingSet.find(uuid) != pendingSet.end();
            // std::cout << "Message with uuid " << uuid << " is new:" << isNew << '\n';
            widget->setNewMessageIndicatorVisible(isNew); // Show or hide the indicator
        }
    }
}

void VTSDialog::updateAckStatuses()
{
    try
    {
        std::map<std::string, int> ackMap = Curler::getAllAckTypes();
        bool mapUpdated = loadAckMap(ackMap);

        if (mapUpdated)
        {
            updateACKStatus();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error at VTS message pulling: " << e.what() << std::endl;
    }
}

void VTSDialog::startVTSLoop()
{
    if (!vtsTimer)
    {
        vtsTimer = new QTimer(this);
        connect(vtsTimer, &QTimer::timeout, this, &VTSDialog::pullNewVTSMessages);
    }

    vtsTimer->start(2500); // Runs every 2.5 seconds
}

void VTSDialog::stopVTSLoop()
{
    if (vtsTimer)
    {
        vtsTimer->stop();
    }
}
