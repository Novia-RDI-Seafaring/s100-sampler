#pragma once

/*!
 * \file floatingtoolbar.h
 * \brief Widget for displaying real-time vessel info. Basically a conning display.
 *
 * Shows speed (knots), heading (degrees), and geographic position (DMS).
 */

#include <QWidget>

namespace Ui {
class ConningWindow;
}

/*!
 * \class ConningWindow
 * \brief A floating UI panel for ship status display.
 *
 * Displays:
 * - Vessel speed (knots)
 * - True heading (degrees)
 * - Latitude/longitude (in DMS format)
 *
 * Applies semi-transparency using QGraphicsOpacityEffect.
 */
class ConningWindow : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor.
     * \param parent Optional parent widget.
     */
    explicit ConningWindow(QWidget *parent = nullptr);
    ~ConningWindow();
    /*!
     * \brief Updates displayed vessel speed.
     * \param speed Value in knots.
     */
    void updateSpeed(double speed);

    /*!
     * \brief Updates displayed heading.
     * \param heading Value in degrees (°).
     */
    void updateHeading(double heading);

    /*!
     * \brief Updates displayed latitude and longitude.
     * \param lat Latitude in decimal degrees.
     * \param lon Longitude in decimal degrees.
     */
    void updatePosition(double lat, double lon);

private:
    Ui::ConningWindow *ui;
};