#include "VTSMessageWidget.hpp"

VTSMessageWidget::VTSMessageWidget(const QString &senderName, const std::vector<std::pair<std::string, std::string>> &displayFields, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(10);
    ackStatus = 0;

    newMessageBar = new QFrame(this);
    newMessageBar->setFixedWidth(4);
    newMessageBar->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFD700, stop:1 #FFA500); border: 1px dashed orange;");
    newMessageBar->hide(); // Initially hidden
    layout->addWidget(newMessageBar);

    QLabel *senderLabel = new QLabel(senderName);

    senderLabel->setStyleSheet("font-weight: bold;");
    senderLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QFontMetrics fm(senderLabel->font());
    int fixedWidth = fm.horizontalAdvance("SHIP") + 35;

    senderLabel->setMinimumWidth(fixedWidth);
    senderLabel->setMaximumWidth(fixedWidth);
    senderLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    layout->addWidget(senderLabel);

    QString html;

    for (const auto &field : displayFields)
    {
        QString label = QString::fromStdString(field.first);
        QString value = QString::fromStdString(field.second);

        if (field.first == "Status")
        {
            QString color = (value == "Confirmed" ? "green" : value == "Requested" ? "orange"
                                                          : value == "Denied"      ? "red"
                                                                                   : "black");

            html += QString("<b>%1:</b> <span style='color:%2;'>%3</span><br>")
                        .arg(label, color, value);
        }
        else
        {
            html += QString("<b>%1:</b> %2<br>").arg(label, value);
        }
    }

    QLabel *messageLabel = new QLabel();
    messageLabel->setTextFormat(Qt::RichText);
    messageLabel->setWordWrap(true);
    messageLabel->setText(html);

    // Make the message label clickable (enable mouse events)
    // messageLabel->setStyleSheet("cursor: pointer;");  // Change cursor to pointer when hovering
    // messageLabel->setAttribute(Qt::WA_Hover, true);    // Enable hover events
    // messageLabel->installEventFilter(this);            // Install event filter to capture mouse clicks
    messageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(messageLabel, 1); // <-- stretches to fill

    if (senderName == "VTS")
    {
        return;
    };

    QHBoxLayout *mainRowLayout = new QHBoxLayout();
    mainRowLayout->setContentsMargins(0, 0, 0, 0);
    mainRowLayout->setSpacing(8);

    // Column for texts
    QVBoxLayout *textColumn = new QVBoxLayout();
    textColumn->setSpacing(4);
    QLabel *seenText = new QLabel("SEEN:");
    QLabel *receivedText = new QLabel("RCVD:");
    textColumn->addWidget(receivedText);
    textColumn->addWidget(seenText);

    // Column for icons
    QVBoxLayout *iconColumn = new QVBoxLayout();
    iconColumn->setSpacing(4);
    seenIconLabel = new QLabel();
    seenIconLabel->setFixedSize(16, 16);

    receivedIconLabel = new QLabel();
    receivedIconLabel->setFixedSize(16, 16);

    iconColumn->addWidget(receivedIconLabel);
    iconColumn->addWidget(seenIconLabel);

    // Assemble columns side-by-side
    mainRowLayout->addLayout(textColumn);
    mainRowLayout->addLayout(iconColumn);

    // Add to your parent layout
    layout->addLayout(mainRowLayout);
};

void VTSMessageWidget::updateStatus(int ackType)
{

    if (ackType <= ackStatus)
    {
        qDebug() << "Ack already up to date" << '\n';
        return;
    }

    QPixmap statusIcon;
    QString iconPath;
    iconPath = "../resources/check_color.svg";
    qDebug() << "updating ack with " << ackType << '\n';

    QLabel *labelpointer = nullptr;
    if (ackType == 1)
    {
        labelpointer = receivedIconLabel;
    }
    else if (ackType == 2)
    {
        labelpointer = seenIconLabel;
    }
    else
    {
        return;
    }

    QSvgRenderer renderer(iconPath);
    if (renderer.isValid())
    {
        statusIcon = QPixmap(16, 16);

        statusIcon.fill(Qt::transparent);
        QPainter painter(&statusIcon);
        painter.setBrush(Qt::BrushStyle::SolidPattern);
        renderer.render(&painter);
        if (statusIcon.isNull())
        {
            qWarning() << "Status icon is null after rendering";
        }
        // Set the rendered icon to the QLabel
        labelpointer->setPixmap(statusIcon.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        // QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
        // labelpointer->setPixmap(icon.pixmap(16, 16));
    }
    else
    {
        qWarning() << "Failed to load SVG icon from" << iconPath;
    }

    ackStatus = ackType;

    if (ackStatus == 2 && (!receivedIconLabel->pixmap(Qt::ReturnByValueConstant::ReturnByValue)))
    {
        qDebug() << "Skipped to ACK 2, updating RCVD" << '\n';
        // QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
        // receivedIconLabel->setPixmap(icon.pixmap(16, 16));
        receivedIconLabel->setPixmap(statusIcon.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
};

int VTSMessageWidget::getAckStatus()
{
    return ackStatus;
}

void VTSMessageWidget::setNewMessageIndicatorVisible(bool visible)
{
    if (newMessageBar)
    {
        newMessageBar->setVisible(visible);
    }
}

#include "moc_VTSMessageWidget.cpp"
