#include "floatingnotificationbox.h"
#include "ui_floatingnotificationbox.h"
#include <QGraphicsOpacityEffect>

FloatingNotificationBox::FloatingNotificationBox(QWidget *parent)
    : QWidget(parent), ui(new Ui::FloatingNotificationBox)
{
    ui->setupUi(this);
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    eff->setOpacity(0.8);
    setGraphicsEffect(eff);
}

FloatingNotificationBox::~FloatingNotificationBox()
{
    delete ui;
}
