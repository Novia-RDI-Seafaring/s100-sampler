#pragma once

#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QLabel>
#include <QSvgRenderer>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <iostream>
#include <vector>
#include <algorithm>

/*!
 * \class VTSMessageWidget
 * \brief VTS message visual widget
 *
 * Containts the visual information of the VTS message. Size, colors, notification styles etc.
 */
class VTSMessageWidget : public QWidget
{
    Q_OBJECT

public:
    VTSMessageWidget(const QString &senderName, const std::vector<std::pair<std::string, std::string>> &messageText, QWidget *parent = nullptr);

    /*!
     * \brief Updates acknowledgement status. Acktype from SECOM standard.
     */
    void updateStatus(int ackType);
    QLabel *iconLabel;
    QLabel *seenIconLabel;
    QLabel *receivedIconLabel;
    QFrame *newMessageBar;

    int getAckStatus();
    void setNewMessageIndicatorVisible(bool visible);

private:
    int ackStatus;
};
