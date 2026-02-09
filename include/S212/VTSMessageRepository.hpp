#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <QList>
#include <QSharedPointer>
#include <SQLiteCpp/SQLiteCpp.h>
#include <unordered_set>
#include <iostream>
#include "VTSMessageWrapper.hpp"

/*!
 * \class VTSMessageRepository
 * \brief Manages local persistence of VTS messages and subscriptions using SQLite.
 *
 * Handles:
 * - Storage/retrieval of messages from the local database
 * - Thread and subscription tracking
 * - ACK status updates
 * - Message insertion and lookups
 */
class VTSMessageRepository
{
public:
    /*!
     * \struct DBMessageRow
     * \brief In-memory representation of a single VTS message row from the database.
     */
    struct DBMessageRow
    {
        std::string uuid;        ///< Unique identifier of the message
        std::string refUUID;     ///< UUID of the referenced message (for threading)
        std::string sender;      ///< Source of the message ("VTS" or "Ship")
        int threadNumber;        ///< Thread association (-1 if not threaded)
        std::string filename;    ///< XML file path
    };

    /*!
     * \brief Constructs and initializes the repository with a database path.
     * \param dbPath Path to the folder containing the SQLite database.
     */
    explicit VTSMessageRepository(const std::string &dbPath);

    /*!
     * \brief Returns all currently active and threaded messages.
     */
    std::vector<DBMessageRow> getActiveMessages();

    /*!
     * \brief Updates sender field for a message UUID.
     */
    void updateSender(const std::string &uuid, Sender sender);

    /*!
     * \brief Marks all unacknowledged VTS messages in the database with acktype = 1 (pending).
     */
    void markUnacknowledgedMessages();

    /*!
     * \brief Sets all VTS messages in a thread to acknowledged (acktype = 2).
     */
    void acknowledgeThread(int threadNumber);

    /*!
     * \brief Retrieves VTS messages in a thread that need acknowledgment.
     */
    std::vector<DBMessageRow> getPendingAcknowledgments(int threadNumber);

    /*!
     * \brief Returns all messages belonging to a specific thread.
     */
    std::vector<DBMessageRow> getMessagesOfThread(int threadNumber);

    /*!
     * \brief Returns all message records, regardless of active status or threading.
     */
    std::vector<DBMessageRow> getMessages();

    /*!
     * \brief Returns all active thread IDs (from subscriptions).
     */
    std::vector<int> getActiveThreadIDs();

    /*!
     * \brief Checks whether a message requires acknowledgment (acktype < 2).
     */
    bool messageNeedsAcknowledgment(const std::string &uuid);

    /*!
     * \brief Sets acknowledgment status for a specific message.
     * \param uuid Message UUID
     * \param acktype 1 = pending, 2 = acknowledged
     */
    void acknowledgeMessage(const std::string &uuid, int acktype);

    /*!
     * \brief Checks if a UUID is already present in the database.
     */
    bool messageInDatabase(const std::string &uuid);

    /*!
     * \brief Inserts a new VTS message into the database.
     */
    void insertMessage(DBMessageRow message);

    /*!
     * \brief Adds or updates a subscription entry for a given thread.
     */
    void insertSubscription(std::string subid, int threadnumber);

    /*!
     * \brief Returns the highest thread number currently in use.
     */
    int highestThreadNumber();

    /*!
     * \brief Retrieves all messages that are not yet assigned to a thread.
     */
    std::vector<DBMessageRow> getUnthreadedMessages();

    /*!
     * \brief Assigns thread numbers to messages based on their RefUUID linkage.
     */
    void linkThreadNumbersByRefID();

    /*!
     * \brief Looks up the thread number of a given RefID (parent UUID).
     * \return Thread number, or -1 if not found.
     */
    int getThreadNumberOfRefID(const std::string &refid);

    /*!
     * \brief Refreshes message active flags using subscription records.
     */
    void updateActiveFlagsFromSubscriptions();

    /*!
     * \brief Deactivates all messages and subscriptions associated with a thread.
     */
    void closeThread(int threadNumber);

    /*!
     * \brief Retrieves the subscription UUID associated with a thread.
     */
    std::string getSubscriptionID(int threadNumber);

    /*!
     * \brief Returns UUID of the most recently received message in a thread.
     */
    std::string getLatestMessageInThread(int threadNumber);

    /*!
     * \brief Returns the XML file path for a given message UUID.
     */
    std::string getFilePath(const std::string &uuid);

private:
    std::unique_ptr<SQLite::Database> vtsDatabase; ///< Internal SQLite database handle
};
