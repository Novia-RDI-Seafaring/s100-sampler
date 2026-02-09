#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <QList>
#include <QSharedPointer>
#include <SQLiteCpp/SQLiteCpp.h>
#include <unordered_set>
#include <QUrl>

#include "VTSParser.hpp"
#include "VTSDigitalInformationMessage.hpp"
#include "VTSMessageWrapper.hpp"
#include "VTSMessageRepository.hpp"
#include "Curler.hpp"

#include "Resources.hpp"
#include <cppcodec/base64_rfc4648.hpp>
#include <nlohmann/json.hpp>

/*!
 * \class VTSDataLayer
 * \brief Manages all VTS message data: parsing, caching, acknowledgment, and server interaction.
 *
 * Handles subscription control, server syncing, and message threading logic.
 * Handles XML parsing of S-212 messages via the parser, database management via VTSMessageRepository.
 *
 */
class VTSDataLayer
{
public:
    /*!
     * \brief Constructs a VTSDataLayer with a message scan path and vessel ID.
     * \param scanPath Filesystem path to scanned message XMLs.
     * \param vesselID ID of the local ship used for sender detection.
     */
    VTSDataLayer(const std::string &scanPath, const std::string &vesselID);

    //! Default constructor, primarily for mocking or placeholder use.
    VTSDataLayer() = default;

    /*!
     * \brief Executes the full VTS message pipeline: scan, thread, load, update.
     */
    void scanAndParseMessages();

    /*!
     * \brief Loads messages from the local DB and parses XML files.
     */
    void loadMessagesFromDatabase();

    /*!
     * \brief Returns the internal message cache.
     */
    const std::unordered_map<std::string, VTSMessageWrapper *> &getMessageCache();

    /*!
     * \brief Subscribes to VTS thread.
     * \param messageUUID UUID of the first message.
     * \param threadNumber New thread ID passed to the database.
     */
    void subscribe(std::string messageUUID, int threadNumber);

    /*!
     * \brief Sends request to unsubscribe from a thread and removes internal subscription record.
     */
    void unsubscribeThread(int threadID);

    /*!
     * \brief Closes an existing message thread (DB and cache).
     */
    void closeThread(int threadID);

    /*!
     * \brief Posts an XML message to the VTS SECOM API.
     */
    void postMessage(std::string messageUUID);

    /*!
     * \brief Sends an acknowledgment (ACK or NACK) for a given message UUID.
     */
    void acknowledgeMessage(std::string messageUUID, int ackType);

    /*!
     * \brief Flags all new messages in DB as acknowledged.
     */
    void assignAckToNewMessages();

    /*!
     * \brief Acknowledges all messages in the given thread.
     */
    void assignAckToThread(int threadNumber);

    /*!
     * \brief Updates a single message as acknowledged.
     */
    void updateAckForMessage(const std::string &uuid);

    /*!
     * \brief Returns UUIDs of all messages in a thread pending acknowledgment.
     */
    std::vector<std::string> getPendingAcksForThread(int threadNumber);

    /*!
     * \brief Checks whether a message still needs acknowledgment.
     */
    bool needsAcknowledgment(const std::string &uuid);

    /*!
     * \brief Returns list of currently active VTS threads.
     */
    std::vector<int> getActiveThreads();

    /*!
     * \brief Attempts to download new VTS messages from the remote server.
     * \return True if any new files were downloaded and processed.
     */
    bool syncWithServer();

    /*!
     * \brief Scans XML files in the data path and inserts them into the DB if not present.
     */
    void preScanMessages();

    /*!
     * \brief Links VTS messages into threads based on their RefID.
     */
    void threader();

    /*!
     * \brief Returns the highest thread number found in the database.
     */
    int highestThreadNumber();

    /*!
     * \brief Retrieves the subscription UUID for a given thread.
     */
    std::string getSubscriptionID(int threadNumber);

    /*!
     * \brief Returns the most recent message UUID in a thread, if available.
     */
    std::optional<std::string> getLatestMessageInThread(int threadNumber);

    /*!
     * \brief Returns the thread number of a given RefID, or -1 if not found.
     */
    int getThreadNumberOfRefID(const std::string &refid);

    /*!
     * \brief Inserts a new message entry into the database.
     */
    void insertMessage(VTSMessageRepository::DBMessageRow message);

    /*!
     * \brief Associates a subscription ID with a thread number.
     */
    void insertSubscription(std::string subid, int threadnumber);

    /*!
     * \brief Constructs a SECOM message JSON payload.
     */
    nlohmann::json makeMessagePayload(
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
        int containerType = 2,
        bool dataProtection = false,
        bool compressionFlag = false,
        int ackRequest = 3,
        bool fromSubscription = false);

    /*!
     * \brief Builds a SECOM acknowledgment (ACK or NACK) payload.
     */
    nlohmann::json makeAcknowledgementPayload(
        const std::string &transactionId,
        const int &ackType,
        const std::string &envelopeCertificate,
        const std::string &digitalSignature,
        const std::string &signatureReference,
        const std::string &thumbprint,
        const std::string &timestampISO,
        const std::optional<int> &nackType = std::nullopt);

    void setEndpoint(QUrl newEndpoint);
    void clearEndpoint();

private:
    std::string dataScanPath; ///< Directory containing XML messages
    std::string ownVesselID;  ///< Local vessel identifier

    std::optional<QUrl> endpoint;

    std::vector<std::string> messageUUIDList;
    std::unordered_map<std::string, VTSMessageWrapper *> messageCache;

    std::unique_ptr<VTSMessageRepository> messageRepo;
    std::unique_ptr<VTSParser> parser;

    /*!
     * \brief Validates the given XML file using xmllint and the S212 schema.
     */
    void validateXML(const std::string &filePath);

    /*!
     * \brief Updates active flags in the DB using current subscription data.
     */
    void updateActiveFlagsFromSubscriptions();

    /*!
     * \brief Reads full content of a file into a string.
     */
    std::string readFile(const std::string &filePath);

    /*!
     * \brief Writes a string to a file.
     */
    void writeFile(const std::string &fileContent, const std::string outputPath);
};
