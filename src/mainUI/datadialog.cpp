#include "datadialog.h"
#include "ui_datadialog.h"
#include <thread>

DataDialog::DataDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::DataDialog)
{
    ui->setupUi(this);
    ui->treeWidget->setColumnCount(2);                              // If you want two columns
    ui->treeWidget->setHeaderLabels({"Title", "Publication Time"}); // Set header labels for clarity
    ui->treeWidget->setSortingEnabled(true);
    // ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // ui->treeWidget->setUniformRowHeights(false);
    // auto *delegate = new WrapDelegate(ui->treeWidget);
    // ui->treeWidget->setItemDelegate(delegate);
    // ui->treeWidget->setItemDelegateForColumn(0, delegate);
    // ui->treeWidget->setItemDelegateForColumn(1, delegate);
    // ui->treeWidget->setWordWrap(true);
    // ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    // ui->treeWidget->setColumnWidth(0, 350); // IMPORTANT

    setModal(false);
    setWindowModality(Qt::NonModal);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &DataDialog::onTreeItemClicked);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &DataDialog::filterTree);
    connect(ui->pullNWButton, &QPushButton::clicked, this, &DataDialog::update);
}

DataDialog::~DataDialog()
{
    delete ui;
}

void DataDialog::pushToTreeWidget(QTreeWidgetItem *item)
{
    if (!item)
    {
        return;
    }

    // Add the root item
    ui->treeWidget->addTopLevelItem(item);
}

void DataDialog::onTreeItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column); // We only care about the item itself

    // Determine which item to use (child or parent)
    QTreeWidgetItem *currentItem = item->parent() ? item->parent() : item;

    // Retrieve the stored S100BaseDataClass pointer
    S100BaseDataClass *dataObject = reinterpret_cast<S100BaseDataClass *>(currentItem->data(0, Qt::UserRole).value<quintptr>());

    if (dataObject)
    {
        emit dataItemClicked(dataObject->getFirstPosition());
    }
}

void DataDialog::showAllItems(QTreeWidgetItem *parentItem)
{
    for (int i = 0; i < parentItem->childCount(); ++i)
    {
        QTreeWidgetItem *childItem = parentItem->child(i);
        childItem->setHidden(false); // Show the item
        showAllItems(childItem);     // Recursive call for children
    }

    parentItem->setHidden(false); // Ensure the parent is visible
}

void DataDialog::filterTree(const QString &searchText)
{
    if (searchText.isEmpty())
    {
        // Show all items if search text is empty
        showAllItems(ui->treeWidget->invisibleRootItem());
    }
    else
    {
        // Filter items based on the search text
        filterItems(searchText);
    }
}

void DataDialog::update()
{
    emit updateButtonClicked();
}

void DataDialog::filterItems(const QString &searchText)
{
    // Iterate through all top-level items in the QTreeWidget
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *topLevelItem = ui->treeWidget->topLevelItem(i);
        bool itemFound = false;

        // Check if any child or descendant matches the search text
        itemFound = checkItemForSearch(topLevelItem, searchText);

        // Show or hide the top-level item based on whether any child or descendant matched
        topLevelItem->setHidden(!itemFound);
    }
}

bool DataDialog::checkItemForSearch(QTreeWidgetItem *item, const QString &searchText)
{
    // Check if any column of the current item contains the search text
    if (item->text(0).contains(searchText, Qt::CaseInsensitive) ||
        item->text(1).contains(searchText, Qt::CaseInsensitive) ||
        item->text(2).contains(searchText, Qt::CaseInsensitive))
    {
        return true; // Match found
    }

    // Recursively check all children of this item
    for (int i = 0; i < item->childCount(); ++i)
    {
        if (checkItemForSearch(item->child(i), searchText))
        {
            return true; // If any child or descendant matches, return true
        }
    }

    return false; // No match found
}

void DataDialog::setTitle(const QString &newTitle)
{
    ui->title->setText(newTitle);
}

void DataDialog::setColumns()
{
    return;
}

void DataDialog::setPullButtonTitle(const QString &newButtonTitle)
{
    ui->pullNWButton->setText(newButtonTitle);
}

void DataDialog::disablePullButton()
{
    ui->pullNWButton->setDisabled(true);
}

void DataDialog::clearTreeWidget()
{
    ui->treeWidget->clear(); // This deletes all top-level items and their children
}

void DataDialog::setUpdateTimeLabel(const QString &timeStr)
{
    ui->updateTime->setText("Last update: " + timeStr);
}
