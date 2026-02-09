#include "getsummarydialog.h"
#include "ui_getsummarydialog.h"

#include "DateTime.hpp"

using json = nlohmann::json;

getSummaryDialog::getSummaryDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::getSummaryDialog)
{
    ui->setupUi(this);
    downloadTimer = new QTimer(this);

    downloadTimer->setInterval(1000 * 60 * 5);
    downloadTimer->stop();

    connect(downloadTimer, &QTimer::timeout, this, &getSummaryDialog::syncWithServer);

    outputFolder = "../data/subAtons";
    // setModal(true);
    // setWindowModality(Qt::WindowModal);
    try
    {
        subDatabase = std::make_unique<SQLite::Database>("../data/DB/aton_subs.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        // Create messages table if it doesn't exist
        subDatabase->exec("CREATE TABLE IF NOT EXISTS subscriptions ("
                          "info_name TEXT PRIMARY KEY, "
                          "sub_uuid TEXT NOT NULL, "
                          "data_uuid TEXT NOT NULL, "
                          "active BOOLEAN DEFAULT 1);");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
}

getSummaryDialog::~getSummaryDialog()
{
    delete ui;
}

// nlohmann binder
inline void from_json(const nlohmann::json &j, SummaryMetadata &s)
{
    s.data_reference = j.value("dataReference", "");
    s.name = j.value("info_name", "");
    s.identifier = j.value("info_identifier", "");
    s.status = j.value("info_status", "");
    s.last_modified_raw = j.value("info_lastModifiedDate", "");
}

std::vector<SummaryMetadata> getSummaryDialog::getServices()
{
    std::vector<SummaryMetadata> rows;

    if (!defaultService)
    {
        std::cerr << "S125 service not declared. Check the MSR menu" << '\n';
        return rows;
    }

    std::string fullURL = defaultService->endpoint.toString().toStdString() + "/v1/object/summary";
    std::cout << fullURL << '\n';

    HttpResponse response;
    try
    {
        response = Curler::getJson(fullURL);
    }
    catch (const std::exception &e)
    {
        std::cerr << "getServices error: " << e.what() << std::endl;
        return rows;
    }

    if (response.body.empty() || response.statusCode == 0)
    {
        std::cerr << "Prolly empty response. No internet?:  " << response.statusCode << '\n';
        return rows;
    }
    // Parse JSON
    if (response.statusCode != 200)
    {
        std::cerr << "Get summary status is weird " << response.statusCode << '\n';
        return rows;
    }

    json jsonData;
    try
    {
        jsonData = json::parse(response.body);
    }
    catch (const json::parse_error &e)
    {
        throw std::runtime_error("Failed to parse JSON response: " + std::string(e.what()));
    }

    const auto *arr = jsonData.contains("summaryObject") && jsonData["summaryObject"].is_array()
                          ? &jsonData["summaryObject"]
                          : nullptr;
    if (!arr)
    { /* handle schema error */
    }

    rows.reserve(arr->size());
    for (const auto &item : *arr)
    {
        rows.push_back(item.get<SummaryMetadata>());
    }

    for (auto summ : rows)
    {
        std::cout << summ.name << ' ' << summ.data_reference << '\n';
    }

    return rows;
}

void getSummaryDialog::updateUI()
{

    const std::vector<SummaryMetadata> rows = getServices();
    ui->summaryTree->clear();
    ui->summaryTree->setColumnCount(3);
    ui->summaryTree->setHeaderLabels({"Name", "Identifier", "Last modified"});

    QSignalBlocker block(ui->summaryTree);
    for (const auto &row : rows)
    {
        auto *it = new QTreeWidgetItem(ui->summaryTree);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        it->setCheckState(0, Qt::Unchecked);
        it->setText(0, QString::fromUtf8(row.name.c_str()));
        it->setText(1, QString::fromUtf8(row.identifier.c_str()));
        const auto dt = QDateTime::fromString(QString::fromUtf8(row.last_modified_raw.c_str()), "yyyyMMdd'T'HHmmss");
        it->setText(2, dt.isValid() ? dt.toString(Qt::ISODate) : QString{});
        it->setData(0, Qt::UserRole, QString::fromStdString(row.data_reference));
        it->setData(0, Qt::UserRole + 1, QString::fromUtf8(row.status.c_str()));
    }

    ui->summaryTree->setSortingEnabled(true);
    ui->summaryTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->summaryTree->header()->setStretchLastSection(true);
}

void getSummaryDialog::on_subButton_pressed()
{
    std::string fullURL = defaultService->endpoint.toString().toStdString() + "/v1/subscription";

    SQLite::Statement getSubs(*subDatabase, "SELECT 1 FROM subscriptions WHERE info_name=? AND active=1 LIMIT 1;");
    SQLite::Statement upsert(*subDatabase,
                             "INSERT INTO subscriptions(sub_uuid, info_name, data_uuid, active) "
                             "VALUES (?, ?, ?, 1) "
                             "ON CONFLICT(info_name) DO UPDATE SET "
                             "  sub_uuid=excluded.sub_uuid, "
                             "  data_uuid=excluded.data_uuid, "
                             "  active=1");

    for (int i = 0; i < ui->summaryTree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *it = ui->summaryTree->topLevelItem(i);
        if (!it)
            continue;

        if (it->checkState(0) == Qt::Checked)
        {
            getSubs.clearBindings();
            upsert.clearBindings();
            auto dataReference = it->data(0, Qt::UserRole).toString().toStdString();

            const QString infoName = it->text(0);
            const QString dataRef = it->data(0, Qt::UserRole).toString();

            getSubs.bind(1, infoName.toUtf8().constData());
            if (getSubs.executeStep())
            {
                std::cerr << infoName.toStdString() << " already subbed to, skipping. Please unsub first" << '\n';
                getSubs.reset();
                continue;
            }

            std::cout << "subbing to " << dataReference << '\n';

            DateTime startTime = DateTime();
            DateTime endTime = startTime;
            endTime.date.day += 1;
            // WONT WORK BTW
            json payload = {
                {"containerType", 0},
                {"dataProductType", "S125"},
                {"dataReference", dataRef.toStdString()},
                //{"productVersion", "1.0.0"},
                {"subscriptionPeriodStart", startTime.toString("ISO", false)},
                {"subscriptionPeriodEnd", endTime.toString("ISO", false)},
                {"callbackEndpoint", ConfigManager::getInstance().getConfig().api_endpoint.value()}};

            HttpResponse response;

            try
            {
                response = Curler::postJson(fullURL, payload.dump());
            }
            catch (const std::exception &e)
            {
                std::cerr << "Atonsub error: " << e.what() << std::endl;
            }

            std::string sub_uuid = "TEMP ID";

            std::cout << "Posting: " << payload.dump() << '\n';
            std::cout << "Response: " << response.body << '\n';

            if (response.statusCode != 200)
            {
                std::cerr << "Funny happen" << '\n';
                return;
            }

            try
            {
                json jsonResponse = json::parse(response.body);
                if (jsonResponse.contains("subscriptionIdentifier") && jsonResponse["subscriptionIdentifier"].is_string())
                {
                    sub_uuid = jsonResponse["subscriptionIdentifier"];
                }
            }
            catch (const json::parse_error &e)
            {
                std::cerr << "JSON parse error: " << e.what() << '\n';
            }

            upsert.bind(1, sub_uuid);
            upsert.bind(2, infoName.toUtf8().constData());
            upsert.bind(3, dataRef.toUtf8().constData());
            upsert.exec();
            upsert.reset();
        }
    }
}

void getSummaryDialog::on_unsubButton_pressed()
{

    std::string fullURL = defaultService->endpoint.toString().toStdString() + "/v1/subscription";

    SQLite::Statement sel(*subDatabase,
                          "SELECT sub_uuid FROM subscriptions WHERE active=1");

    SQLite::Transaction tx(*subDatabase);
    SQLite::Statement markInactive(*subDatabase,
                                   "UPDATE subscriptions SET active=0 WHERE sub_uuid=?");

    while (sel.executeStep())
    {
        const std::string sub_uuid = sel.getColumn(0).getString();

        nlohmann::json payload = {{"subscriptionIdentifier", sub_uuid}};

        HttpResponse response;

        try
        {
            response = Curler::deleteJson(fullURL, payload.dump());
        }
        catch (const std::exception &e)
        {
            std::cerr << "aton sub del error: " << e.what() << std::endl;
        }

        if (response.statusCode != 200)
        {
            std::cerr << "Funny happened at unsub" << '\n';
            std::cerr << response.body << '\n';
            continue;
        }

        markInactive.bind(1, sub_uuid);
        markInactive.exec();
        markInactive.reset();
    }
    tx.commit();
}

std::vector<std::string> getSummaryDialog::getActiveSubs()
{
    std::vector<std::string> activeSubs;
    SQLite::Statement sel(*subDatabase,
                          "SELECT sub_uuid FROM subscriptions WHERE active=1");

    while (sel.executeStep())
    {
        const std::string sub_uuid = sel.getColumn(0).getString();
        activeSubs.push_back(sub_uuid);
    }
    return activeSubs;
}

bool getSummaryDialog::syncWithServer()
{
    auto subs = getActiveSubs();
    if (subs.size() == 0)
    {
        return false;
    }

    try
    {
        std::vector<std::string> serverUuids = Curler::getAtonFileList();
        if (serverUuids.empty())
            return false;

        std::unordered_set<std::string> localUuidSet = scanFolder();

        bool newFilesProcessed = false; // Track whether new files are processed
        const std::string baseUrl = ConfigManager::getInstance().getConfig().api_endpoint.value();

        for (const auto &uuid : serverUuids)
        {

            if (localUuidSet.find(uuid) != localUuidSet.end())
                continue;

            const std::string url = baseUrl + "/v1/atons/" + uuid;

            auto response = Curler::getJson(url);
            if (response.body.empty())
            {
                continue;
            }
            const std::string outputFile = outputFolder + "/" + uuid + ".xml";
            writeFile(response.body, outputFile);

            newFilesProcessed = true;
        }
        if (!newFilesProcessed) // No new files to process
            return false;

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "syncWithServer error: " << e.what() << std::endl;
        return false;
    }
}

void getSummaryDialog::writeFile(const std::string &fileContent, const std::string outputPath)
{
    std::ofstream file(outputPath, std::ios::out | std::ios::binary);
    if (!file)
        throw std::runtime_error("Unable to create file: " + outputPath);

    file << fileContent;
    file.close();
}

// Reads files from the folder
std::unordered_set<std::string> getSummaryDialog::scanFolder()
{
    std::unordered_set<std::string> files;
    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(outputFolder))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".xml")
                continue;

            files.insert(entry.path().stem().string());
            std::cout << " scanned " << entry.path().stem().string() << '\n';
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Pre-scan error: " << e.what() << std::endl;
    }
    return files;
}
void getSummaryDialog::on_getSummaryButton_pressed()
{
    updateUI();
}
