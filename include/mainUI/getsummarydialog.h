#ifndef GETSUMMARYDIALOG_H
#define GETSUMMARYDIALOG_H

#include <QDialog>
#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <iostream>
#include "Curler.hpp"
#include "S100Service.hpp"
#include <nlohmann/json.hpp>
#include <QDateTime>
#include "Resources.hpp"
#include <unordered_set>
#include <QTimer>
struct SummaryMetadata
{
    std::string data_reference;
    std::string name;
    std::string identifier;
    std::string status;
    std::string last_modified_raw; // exp: "20250430T082553"
};

/*!
 * \file getsummarydialog.h
 * \brief Widget for pulling and displaying AtoN getSummary results. Made for a specific test; just hacked together.
 *
 */

namespace Ui
{
    class getSummaryDialog;
}

class getSummaryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit getSummaryDialog(QWidget *parent = nullptr);
    std::optional<S100Service> defaultService;

    ~getSummaryDialog();
    std::vector<SummaryMetadata> getServices();
    void updateUI();
    bool syncWithServer();
private slots:
    void on_subButton_pressed();
    void on_unsubButton_pressed();
    void on_getSummaryButton_pressed();

private:
    Ui::getSummaryDialog *ui;
    std::unique_ptr<SQLite::Database> subDatabase; ///< Internal SQLite database handle
    void writeFile(const std::string &fileContent, const std::string outputPath);
    std::unordered_set<std::string> scanFolder();
    std::vector<std::string> getActiveSubs();
    std::string outputFolder = "";
    QTimer *downloadTimer = nullptr;
};

#endif // GETSUMMARYDIALOG_H
