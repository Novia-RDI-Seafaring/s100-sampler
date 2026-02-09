#pragma once

/*!
 * \file datadialog.h
 * \brief Defines the DataDialog class for displaying hierarchical data in a tree view.
 *
 * The dialog integrates with Qt UI and external data sources to show structured items
 * such as navigation warnings. It supports user interaction, item filtering,
 * dynamic UI updates, and emits signals on user actions.
 */

#include <QDialog>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include "NAVWARNPreamble.hpp"
#include "Curler.hpp"
#include "Resources.hpp"

namespace Ui
{
    class DataDialog;
}

/*!
 * \class DataDialog
 * \brief UI dialog for rendering and managing tree-based data content.
 *
 * Features:
 * - Adds structured data to a QTreeWidget
 * - Filters items based on user input
 * - Reacts to tree item clicks
 * - Supports dynamic title, button labels, and update info
 * - Emits signals to notify external handlers of user actions
 *
 * Designed to be used with data objects that inherit from S100BaseDataClass,
 * specifically emitting location information for mapping or visualization.
 */
class DataDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Constructs the dialog and sets up signal-slot connections.
     */
    explicit DataDialog(QWidget *parent = nullptr);
    ~DataDialog();

    /*!
     * \brief Adds a top-level QTreeWidgetItem to the tree view.
     * \param item Pointer to the item to be added.
     */
    void pushToTreeWidget(QTreeWidgetItem *item);

    /*!
     * \brief Sets the dialog window title.
     * \param newTitle The title as a std::string.
     */
    void setTitle(const QString& newTitle);

    /*!
     * \brief Placeholder for tree column setup. Currently unused.
     */
    void setColumns();

    /*!
     * \brief Updates the label of the pull/update button.
     * \param newButtonTitle New label for the button.
     */
    void setPullButtonTitle(const QString& newButtonTitle);

    /*!
     * \brief Disables the pull button to prevent further user interaction.
     */
    void disablePullButton();

    /*!
     * \brief Clears all items from the tree widget.
     *
     * Frees memory for top-level and child items.
     */
    void clearTreeWidget();

    /*!
     * \brief Sets the label showing when the data was last updated.
     * \param timeStr Timestamp string to show.
     */
    void setUpdateTimeLabel(const QString &timeStr);

signals:
    /*!
     * \brief Emitted when the user selects an item in the tree.
     * \param Pointer to the geographic position object from the selected item.
     */
    void dataItemClicked(DirectPosition *pos);

    /*!
     * \brief Emitted when the user clicks the update/pull button.
     */
    void updateButtonClicked();

private slots:
    /*!
     * \brief Filters visible items based on search text input.
     * \param searchText Text to match against tree item contents.
     */
    void filterTree(const QString &searchText);

    /*!
     * \brief Handles logic when update button is clicked.
     *
     * Emits the updateButtonClicked signal which triggers data download down the line.
     */
    void update();

    /*!
     * \brief Called when a tree item is clicked by the user.
     * \param item Pointer to the clicked QTreeWidgetItem.
     * \param column Index of the clicked column (unused).
     */
    void onTreeItemClicked(QTreeWidgetItem *item, int column);

private:
    //! Pointer to the auto-generated UI form.
    Ui::DataDialog *ui;

    /*!
     * \brief Filters all tree items based on the given search text.
     * \param searchText Filter string.
     */
    void filterItems(const QString &searchText);

    /*!
     * \brief Checks if the item or any of its children match the search text.
     * \param item QTreeWidgetItem to check.
     * \param searchText Text to match.
     * \return true if match found, false otherwise.
     */
    bool checkItemForSearch(QTreeWidgetItem *item, const QString &searchText);

    /*!
     * \brief Recursively shows all items in the tree.
     * \param parentItem Root item to start recursion from.
     */
    void showAllItems(QTreeWidgetItem *parentItem);
};
