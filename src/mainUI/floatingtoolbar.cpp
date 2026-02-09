#include "floatingtoolbar.h"
#include "ui_floatingtoolbar.h"
#include <QGraphicsOpacityEffect>

ConningWindow::ConningWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConningWindow)
{
    ui->setupUi(this);
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    eff->setOpacity(0.8);
    setGraphicsEffect(eff);
}

ConningWindow::~ConningWindow()
{
    delete ui;
}

void ConningWindow::updateSpeed(double speed)
{
    if (ui->speedLabel) {
        ui->speedLabel->setText(QString::number(speed, 'f', 2) + " knots");
    }
}

void ConningWindow::updateHeading(double heading)
{
    if (ui->headingLabel) {
        ui->headingLabel->setText(QString::number(heading, 'f', 1) + "°");
    }
}


QString convertToDMS(double value, char positive, char negative)
{
    // Determine the direction (N/S or E/W)
    char direction = (value >= 0) ? positive : negative;
    value = qAbs(value);  // Use absolute value for conversion
    
    // Degrees, minutes, and seconds calculation
    int degrees = static_cast<int>(value);
    double minutesDecimal = (value - degrees) * 60;

    // Ensure latitude (N/S) has space padding and longitude (E/W) has 3 digits
    QString degreeString;
    if (direction == 'E' || direction == 'W') {
        degreeString = QString::number(degrees).rightJustified(3, '0'); // Longitude: 3 digits (021, 123)
    } else {
        degreeString = QString::number(degrees).rightJustified(3, ' '); // Latitude: 2 digits, space padded (" 9", "60")
    }

    // Minutes and seconds formatting
    QString minutesString = QString::number(minutesDecimal, 'f', 2);
    // Format final output
    return QString("%1° %2' %3")
            .arg(degreeString)  
            .arg(minutesString)             
            .arg(direction);
}

void ConningWindow::updatePosition(double lat, double lon)
{
    if (ui->positionLabel) {
        QString latDMS = convertToDMS(lat, 'N', 'S');
        QString lonDMS = convertToDMS(lon, 'E', 'W');
        ui->positionLabel->setText(latDMS + "\n" + lonDMS);
    }
}
