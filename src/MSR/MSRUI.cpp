#include "MSRUI.hpp"
#include "ui_MSRUI.h"
#include <iostream>
MSRUI::MSRUI(QWidget *parent)
    : QDialog(parent), ui(new Ui::MSRUI)
{
    ui->setupUi(this);
    QTreeWidget *tree = ui->serviceListWidget;
    tree->setColumnCount(3);
    tree->setHeaderLabels({"Service", "Data Product", ""});

    tree->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    // Pick a tight width; compute from text + frame/padding if you want
    const int w = tree->fontMetrics().horizontalAdvance(QStringLiteral("Test Connection")) + 16; // +padding
    tree->setColumnWidth(2, qMax(40, w));                                                        // e.g. 48?64 looks good

    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch); // or ResizeToContents
    connect(ui->serviceListWidget, &QTreeWidget::itemChanged,
            this, &MSRUI::onServiceCheckboxStateChanged);

    QPushButton *restoreBtn = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);

    if (restoreBtn)
    {
        restoreBtn->setText("Clear Database");
        connect(restoreBtn, &QPushButton::clicked,
                this, &MSRUI::onRestoreDefaults);
    }
    setWindowModality(Qt::WindowModal);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
}

MSRUI::~MSRUI()
{
    delete ui;
}

void MSRUI::loadProviders(QVector<S100Provider> providers)
{
    ui->serviceProvidersComboBox->clear();

    for (auto provider : providers)
    {
        ui->serviceProvidersComboBox->addItem(provider.name, QVariant::fromValue(provider));
    }

    connect(ui->loadServicesButton, &QPushButton::clicked, this, [this]
            {
    const QVariant v = ui->serviceProvidersComboBox->currentData();
    if (!v.isValid() || !v.canConvert<S100Provider>()) return;

    const S100Provider provider = v.value<S100Provider>();
    providerCall(provider); });
};

void MSRUI::loadServices(QVector<S100Service> services)
{
    QTreeWidget *tree = ui->serviceListWidget;
    QMap<QString, QTreeWidgetItem *> orgParents;

    // Map existing parents
    const int topLevelCount = tree->topLevelItemCount();
    for (int i = 0; i < topLevelCount; ++i)
    {
        QTreeWidgetItem *item = tree->topLevelItem(i);
        orgParents[item->text(0)] = item;
    }

    for (const auto &p : services)
    {
        QTreeWidgetItem *parent = nullptr;

        // Reuse or create parent
        if (orgParents.contains(p.orgId))
        {
            parent = orgParents[p.orgId];
        }
        else
        {
            parent = new QTreeWidgetItem(tree);
            parent->setText(0, p.orgId);
            parent->setFlags(parent->flags() |
                             Qt::ItemIsUserCheckable |
                             Qt::ItemIsTristate);
            parent->setCheckState(0, Qt::Unchecked);
            orgParents[p.orgId] = parent;
        }

        // Check for existing child by name
        QTreeWidgetItem *existingChild = nullptr;
        for (int j = 0; j < parent->childCount(); ++j)
        {
            if (parent->child(j)->text(0) == p.name)
            {
                existingChild = parent->child(j);
                break;
            }
        }

        if (existingChild)
        {
            // Overwrite existing service data
            existingChild->setData(0, Qt::UserRole, QVariant::fromValue(p));
            existingChild->setCheckState(0, p.active ? Qt::Checked : Qt::Unchecked);
            if (auto *w = tree->itemWidget(existingChild, 1))
            {
                if (auto *btn = qobject_cast<QToolButton *>(w))
                {
                    btn->setProperty("endpoint", p.endpoint);
                    btn->setToolTip(p.endpoint.toDisplayString());
                }
            }
        }
        else
        {
            // Add new child
            auto *child = new QTreeWidgetItem(parent);
            child->setText(0, p.name);
            child->setData(0, Qt::UserRole, QVariant::fromValue(p));
            child->setFlags(child->flags() | Qt::ItemIsUserCheckable);
            child->setCheckState(0, p.active ? Qt::Checked : Qt::Unchecked);

            child->setText(1, p.dataProductType);

            auto *btn = new QToolButton();
            btn->setText("Test Connection");
            btn->setToolButtonStyle(Qt::ToolButtonTextOnly); // no big frame
            btn->setAutoRaise(false);                        // hover highlight only
            // btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            // 1btn->setFixedSize(50, 20); // tweak width/height to your taste
            btn->setProperty("endpoint", p.endpoint);
            btn->setToolTip(p.endpoint.toDisplayString());
            connect(btn, &QToolButton::clicked, this, &MSRUI::onPingClicked);
            tree->setItemWidget(child, 2, btn);
        }
    }

    tree->expandAll();
}

void MSRUI::updatePingStatus(const QUrl &endpoint, bool success)
{
    QTreeWidget *tree = ui->serviceListWidget;

    for (int i = 0; i < tree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *parent = tree->topLevelItem(i);

        for (int j = 0; j < parent->childCount(); ++j)
        {
            QTreeWidgetItem *child = parent->child(j);

            // recover your service payload
            S100Service svc = child->data(0, Qt::UserRole).value<S100Service>();
            if (svc.endpoint == endpoint)
            {
                if (auto *btn = qobject_cast<QToolButton *>(tree->itemWidget(child, 2)))
                {
                    btn->setStyleSheet(success
                                           ? "QToolButton { background-color: rgba(0,128,0,160); }"
                                           : "QToolButton { background-color: rgba(220,0,0,160); }");
                }
                return;
            }
        }
    }
}

void MSRUI::onPingClicked()
{
    auto *btn = qobject_cast<QToolButton *>(sender());
    if (!btn)
        return;

    QUrl ep = btn->property("endpoint").toUrl();
    emit pingSignal(ep);
}

void MSRUI::onRestoreDefaults()
{

    QTreeWidget *tree = ui->serviceListWidget;
    QVector<S100Service> activeServices;
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *parent = tree->topLevelItem(i);

        for (int j = 0; j < parent->childCount(); ++j)
        {
            QTreeWidgetItem *child = parent->child(j);

            S100Service svc = child->data(0, Qt::UserRole).value<S100Service>();
            if (svc.active == true || child->checkState(0) == Qt::Checked)
            {
                activeServices.push_back(svc);
            }
        }
    }
    for (auto &service : activeServices)
    {
        service.active = false;
        emit serviceChange(service);
    }

    emit clearDatabase();
    tree->clear();
}

void MSRUI::onServiceCheckboxStateChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0)
        return;

    auto variant = item->data(0, Qt::UserRole);
    if (!variant.canConvert<S100Service>())
        return;

    S100Service service = variant.value<S100Service>();

    if (item->checkState(0) == Qt::Checked)
    {
        service.active = true;
    }
    else
    {
        service.active = false;
    }
    emit serviceChange(service);
}

void MSRUI::populateTempData()
{
}

#include "moc_MSRUI.cpp"
