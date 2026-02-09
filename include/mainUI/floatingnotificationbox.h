#pragma once

/*!
 * \file floatingnotificationbox.h
 * \brief Defines a lightweight, semi-transparent notification widget.
 *
 * This widget overlays on top of other UI elements to present temporary messages or indicators.
 * Visual effects include adjustable opacity. It is intended for passive alerts.
 * The NotificationBox is later passed to the VTS dialog, which handles its logic.
 */

#include <QWidget>

namespace Ui {
class FloatingNotificationBox;
}

/*!
 * \class FloatingNotificationBox
 * \brief A transparent widget used to display floating notifications.
 *
 * Applies a static opacity effect on initialization. Can be used to show status updates,
 * non-blocking alerts, or transient information to the user in a non-intrusive way.
 */
class FloatingNotificationBox : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingNotificationBox(QWidget *parent = nullptr);

    //! Pointer to the UI form, exposed for direct control.
    Ui::FloatingNotificationBox *ui;

private:
    ~FloatingNotificationBox();
};