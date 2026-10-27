#include "VTSDataLayer.hpp"

using json = nlohmann::json;

VTSDataLayer::VTSDataLayer(const std::string &scanPath, const std::string &vesselID) : dataScanPath(scanPath), ownVesselID(vesselID)
{
    parser = std::make_unique<VTSParser>();

    auto dataDir = std::filesystem::path("../data/VTS/");

    if (!std::filesystem::exists(dataDir))
    {
        std::cout << "Data directory missing: " << dataDir << "\n";
        std::filesystem::create_directories(dataDir);
    }

    messageRepo = std::make_unique<VTSMessageRepository>("../data/DB/");
}

void VTSDataLayer::assignAckToNewMessages()
{
    messageRepo->markUnacknowledgedMessages();
}

void VTSDataLayer::assignAckToThread(int threadNumber)
{
    messageRepo->acknowledgeThread(threadNumber);
}

std::vector<std::string> VTSDataLayer::getPendingAcksForThread(int threadNumber)
{
    auto rows = messageRepo->getPendingAcknowledgments(threadNumber);
    std::vector<std::string> uuids;

    for (auto row : rows)
    {
        uuids.push_back(row.uuid);
    }

    return uuids;
}

std::vector<int> VTSDataLayer::getActiveThreads()
{
    std::vector<int> threads = messageRepo->getActiveThreadIDs();
    return threads;
}

bool VTSDataLayer::needsAcknowledgment(const std::string &uuid)
{
    return messageRepo->messageNeedsAcknowledgment(uuid);
}

void VTSDataLayer::updateAckForMessage(const std::string &uuid)
{
    messageRepo->acknowledgeMessage(uuid, 2);
}

void VTSDataLayer::loadMessagesFromDatabase()
{
    try
    {
        auto messages = messageRepo->getActiveMessages();

        for (const auto &row : messages)
        {
            if (messageCache.find(row.uuid) != messageCache.end())
                continue;

            std::string filePath = dataScanPath + "/" + row.filename;
            if (!std::filesystem::exists(filePath))
            {
                std::cerr << "Missing file for UUID " << row.uuid << ": " << filePath << "\n";
                continue;
            }

            pugi::xml_document doc;
            if (!doc.load_file(filePath.c_str()))
            {
                std::cerr << "Failed to parse XML for UUID " << row.uuid << "\n";
                continue;
            }

            VTSDigitalInformationMessage *parsed = parser->readVTSDigitalInformationMessage(doc);
            if (!parsed)
            {
                std::cerr << "Invalid VTS message: " << row.uuid << "\n";
                continue;
            }

            Sender sender = Sender::Ship;
            if (parsed->reportedBy)
                sender = (parsed->reportedBy.value() == ownVesselID) ? Sender::Ship : Sender::VTS;

            auto *wrapper = new VTSMessageWrapper(parsed, sender);
            wrapper->uuid = row.uuid;
            wrapper->threadNumber = row.threadNumber;
            wrapper->filename = row.filename;
            messageCache[row.uuid] = wrapper;

            messageRepo->updateSender(row.uuid, sender);
        }

        std::cout << "Loaded " << messageCache.size() << " messages from DB.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading messages from DB: " << e.what() << std::endl;
    }
}

void VTSDataLayer::validateXML(const std::string &filePath)
{
    // std::string command = "xmllint --noout --valid " + filePath;
    std::string command = "xmllint --noout --schema ../schemas/S212.xsd " + filePath;
    int result = std::system(command.c_str());

    if (result == 0)
    {
        std::cout << "XML file is valid.\n";
    }
    else
    {
        std::cerr << "XML validation failed!\n";
    }
}

// Reads new files from the folder, reads their refid and uuid and inserts to database.
void VTSDataLayer::preScanMessages()
{
    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(dataScanPath))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".xml")
                continue;

            pugi::xml_document doc;
            if (!doc.load_file(entry.path().string().c_str()))
                continue;

            std::string uuid = parser->readUUID(doc);
            std::string refID = parser->readRefID(doc);
            if (uuid.empty())
                continue;

            // Skip uuid if in DB
            if (messageRepo->messageInDatabase(uuid))
            {
                continue;
            }

            VTSMessageRepository::DBMessageRow message;
            message.uuid = uuid;
            message.refUUID = refID;
            message.threadNumber = -1;
            message.sender = "";
            message.filename = entry.path().filename().string();

            messageRepo->insertMessage(message);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Pre-scan error: " << e.what() << std::endl;
    }
}

void VTSDataLayer::threader()
{
    try
    {
        messageRepo->linkThreadNumbersByRefID();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Threader error: " << e.what() << std::endl;
    }
}

void VTSDataLayer::scanAndParseMessages()
{
    preScanMessages();
    threader();
    loadMessagesFromDatabase();
    updateActiveFlagsFromSubscriptions();
}

void VTSDataLayer::updateActiveFlagsFromSubscriptions()
{
    try
    {
        messageRepo->updateActiveFlagsFromSubscriptions();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Active update error: " << e.what() << std::endl;
    }
}

int VTSDataLayer::highestThreadNumber()
{
    try
    {
        return messageRepo->highestThreadNumber();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Active update error: " << e.what() << std::endl;
    }
    return 0;
}

const std::unordered_map<std::string, VTSMessageWrapper *> &VTSDataLayer::getMessageCache()
{
    return messageCache;
}

void VTSDataLayer::closeThread(int threadID)
{
    try
    {

        // Retrieve affected UUIDs from the repository
        auto messagesToRemove = messageRepo->getMessagesOfThread(threadID);

        // Delegate DB work to repository
        messageRepo->closeThread(threadID);

        // Remove from cache
        for (const auto &message : messagesToRemove)
        {
            messageCache.erase(message.uuid);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error closing thread in data layer: " << e.what() << std::endl;
    }
}

void VTSDataLayer::unsubscribeThread(int threadNumber)
{
    try
    {
        std::string subscriptionUUID = getSubscriptionID(threadNumber);
        json payload = {{"subscriptionIdentifier", subscriptionUUID}};
        std::string jsonPayload = payload.dump();
        std::string targetEndpoint = (endpoint.value().toString() + "/v1/subscription").toStdString();
        const auto &cfg = ConfigManager::getInstance().getConfig();
        auto response = Curler::deleteJson(targetEndpoint, jsonPayload, cfg.cert, cfg.key);
        qDebug() << "Unsubscribed from thread " << threadNumber << ", UUID: " << QString::fromStdString(subscriptionUUID);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error while unsubscribing: " << e.what() << std::endl;
    }
}

void VTSDataLayer::subscribe(std::string messageUUID, int threadNumber)
{
    std::string targetEndpoint = (endpoint.value().toString() + "/v1/subscription").toStdString();

    const auto &cfg = ConfigManager::getInstance().getConfig();
    const auto &certOpt = cfg.cert;
    const auto &keyOpt = cfg.key;

    json payload = {
        {"containerType", 2},
        {"dataProductType", "S212"},
        {"dataReference", messageUUID},
        {"productVersion", "1.0.0"},
        {"callbackEndpoint", cfg.api_endpoint.value()}};

    auto response = Curler::postJson(targetEndpoint, payload.dump(), certOpt, keyOpt);
    std::cout << "Response was " << response.statusCode << " " << response.body << '\n';
    if (response.statusCode != 200)
    {
        std::string errormessage = "Error in subscribing: " + std::to_string(response.statusCode) + ' ' + response.body;
        throw std::runtime_error(errormessage);
    }
    // Parse the JSON response
    try
    {
        json jsonResponse = json::parse(response.body);
        if (jsonResponse.contains("subscriptionIdentifier") && jsonResponse["subscriptionIdentifier"].is_string())
        {
            insertSubscription(jsonResponse["subscriptionIdentifier"], threadNumber);
        }
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parse error: " << e.what() << '\n';
    }
}

void VTSDataLayer::postMessage(std::string messageUUID)
{
    std::string targetEndpoint = (endpoint.value().toString() + "/v1/object").toStdString();

    auto path = messageRepo->getFilePath(messageUUID);
    std::string xmlContent = readFile(dataScanPath + "/" + path);
    std::string basedContent = cppcodec::base64_rfc4648::encode(xmlContent);
    DateTime timestamp = DateTime();
    json payload = makeMessagePayload(basedContent, messageUUID, "AAA", "cert", "cert2", "thumbprint", "S212", "dsa", "SECOM", timestamp.toString("ISO"));
    std::cout << "Posting a message " << payload.dump();

    const auto &cfg = ConfigManager::getInstance().getConfig();

    auto response = Curler::postJson(targetEndpoint, payload.dump(), cfg.cert, cfg.key);
    std::cout << "Response is " << response.statusCode << " " << response.body << '\n';
}

void VTSDataLayer::acknowledgeMessage(std::string messageUUID, int ackType)
{
    if (!endpoint)
    {
        return;
    }

    DateTime timestamp = DateTime();
    json payload = makeAcknowledgementPayload(messageUUID, ackType, "cert", "sign", "ref", "thumbprint", timestamp.toString("ISO"));
    std::string targetEndpoint = (endpoint.value().toString() + "/v1/acknowledgement").toStdString();

    const auto &cfg = ConfigManager::getInstance().getConfig();
    Curler::postJson(targetEndpoint, payload.dump(), cfg.cert, cfg.key);
}

bool VTSDataLayer::syncWithServer()
{
    try
    {
        std::vector<std::string> serverUuids = Curler::getFileList();
        if (serverUuids.empty())
            return false;

        std::vector<VTSMessageRepository::DBMessageRow> localMessages = messageRepo->getMessages();
        std::unordered_set<std::string> localUuidSet;
        std::unordered_set<std::string> newUuids;

        for (const auto &row : localMessages)
            localUuidSet.insert(row.uuid);

        bool newFilesProcessed = false; // Track whether new files are processed
        const std::string baseUrl = ConfigManager::getInstance().getConfig().api_endpoint.value();

        for (const auto &uuid : serverUuids)
        {

            if (localUuidSet.find(uuid) != localUuidSet.end())
                continue;

            const std::string url = baseUrl + "/v1/files/" + uuid;

            auto response = Curler::getJson(url);
            if (response.body.empty())
            {
                continue;
            }
            const std::string outputFile = dataScanPath + "/" + uuid + ".xml";
            writeFile(response.body, outputFile);

            newFilesProcessed = true;
            // Make a check that succeeds
            acknowledgeMessage(uuid, 1);
            newUuids.insert(uuid);
        }
        if (!newFilesProcessed) // No new files to process
            return false;

        // Once all new files have been downloaded and acknowledged, run preScanMessages to insert the new messages into the DB
        preScanMessages();

        // Now update the acktype for messages that have been processed in the preScan
        for (const auto &uuid : newUuids)
        {
            messageRepo->acknowledgeMessage(uuid, 1);
        }

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "syncWithServer error: " << e.what() << std::endl;
        return false;
    }
}

std::string VTSDataLayer::getSubscriptionID(int threadNumber)
{
    return messageRepo->getSubscriptionID(threadNumber);
};

std::optional<std::string> VTSDataLayer::getLatestMessageInThread(int threadNumber)
{
    std::string latestUUID = messageRepo->getLatestMessageInThread(threadNumber);
    if (latestUUID == "")
    {
        return std::nullopt;
    }
    return latestUUID;
}

int VTSDataLayer::getThreadNumberOfRefID(const std::string &refid)
{
    return messageRepo->getThreadNumberOfRefID(refid);
};

void VTSDataLayer::insertMessage(VTSMessageRepository::DBMessageRow message)
{
    messageRepo->insertMessage(message);
};

void VTSDataLayer::insertSubscription(std::string subid, int threadnumber)
{
    messageRepo->insertSubscription(subid, threadnumber);
}

nlohmann::json VTSDataLayer::makeMessagePayload(
    const std::string &base64Content,
    const std::string &uuid,
    const std::string &signatureValue,
    const std::string &publicCertificate,
    const std::string &signatureCertificate,
    const std::string &thumbprint,
    const std::string &dataProductType,
    const std::string &signatureReference,
    const std::string &protectionScheme,
    const std::string &signatureTimeISO,
    int containerType,
    bool dataProtection,
    bool compressionFlag,
    int ackRequest,
    bool fromSubscription)
{
    return {
        {"envelope", {{"data", base64Content}, {"containerType", containerType}, {"dataProductType", dataProductType}, {"exchangeMetadata", {{"dataProtection", dataProtection}, {"protectionScheme", protectionScheme}, {"digitalSignatureReference", signatureReference}, {"digitalSignatureValue", {{"publicCertificate", publicCertificate}, {"digitalSignature", signatureValue}}}, {"compressionFlag", compressionFlag}}}, {"fromSubscription", fromSubscription}, {"ackRequest", ackRequest}, {"transactionIdentifier", uuid}, {"envelopeSignatureCertificate", signatureCertificate}, {"envelopeRootCertificateThumbprint", thumbprint}, {"envelopeSignatureTime", signatureTimeISO}}},
        {"envelopeSignature", signatureValue}};
}

nlohmann::json VTSDataLayer::makeAcknowledgementPayload(
    const std::string &transactionId,
    const int &ackType,
    const std::string &envelopeCertificate,
    const std::string &digitalSignature,
    const std::string &signatureReference,
    const std::string &thumbprint,
    const std::string &timestampISO,
    const std::optional<int> &nackType)
{
    nlohmann::json envelopeAckObject = {
        {"createdAt", timestampISO},
        {"envelopeCertificate", envelopeCertificate},
        {"transactionIdentifier", transactionId},
        {"ackType", ackType},
        {"envelopeRootCertificateThumbprint", thumbprint},
        {"digitalSignatureReference", signatureReference},
        {"envelopeSignatureTime", timestampISO}};

    if (nackType.has_value())
    {
        envelopeAckObject["nackType"] = *nackType;
    }

    return {
        {"envelope", envelopeAckObject},
        {"digitalSignature", digitalSignature}};
}

void VTSDataLayer::setEndpoint(QUrl newEndpoint)
{
    endpoint = newEndpoint;
}

void VTSDataLayer::clearEndpoint()
{
    endpoint.reset();
}

std::string VTSDataLayer::readFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file)
        throw std::runtime_error("Error: File not found.");
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
};

void VTSDataLayer::writeFile(const std::string &fileContent, const std::string outputPath)
{
    std::ofstream file(outputPath, std::ios::out | std::ios::binary);
    if (!file)
        throw std::runtime_error("Unable to create file: " + outputPath);

    file << fileContent;
    file.close();
}