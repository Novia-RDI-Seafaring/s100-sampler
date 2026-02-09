#include "VTSMessageRepository.hpp"

VTSMessageRepository::VTSMessageRepository(const std::string &dbPath)
{

    try
    {
        vtsDatabase = std::make_unique<SQLite::Database>("../data/DB/vts_messages.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        // Create messages table if it doesn't exist
        vtsDatabase->exec("CREATE TABLE IF NOT EXISTS vts_messages ("
                          "uuid TEXT PRIMARY KEY, "
                          "ref_uuid TEXT, "
                          "thread_number INTEGER, "
                          "sender TEXT, "
                          "acktype INTEGER, "
                          "filename TEXT, "
                          "active BOOLEAN DEFAULT 1);");

        // Create subscriptions table if it doesn't exist
        vtsDatabase->exec("CREATE TABLE IF NOT EXISTS subscriptions ("
                          "uuid_sub TEXT PRIMARY KEY, "
                          "thread_number INTEGER, "
                          "active BOOLEAN);");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
}

std::vector<VTSMessageRepository::DBMessageRow> VTSMessageRepository::getActiveMessages()
{
    std::vector<DBMessageRow> results;

    SQLite::Statement query(*vtsDatabase,
                            "SELECT uuid, ref_uuid, thread_number, filename "
                            "FROM vts_messages WHERE active = 1 AND thread_number != -1;");

    while (query.executeStep())
    {
        DBMessageRow row;
        row.uuid = query.getColumn(0).getString();
        row.refUUID = query.getColumn(1).isNull() ? "" : query.getColumn(1).getString();
        row.threadNumber = query.getColumn(2).getInt();
        row.filename = query.getColumn(3).getString();
        results.push_back(std::move(row));
    }

    return results;
}

void VTSMessageRepository::updateSender(const std::string &uuid, Sender sender)
{
    SQLite::Statement stmt(*vtsDatabase,
                           "UPDATE vts_messages SET sender = ? WHERE uuid = ?;");
    stmt.bind(1, sender == Sender::Ship ? "Ship" : "VTS");
    stmt.bind(2, uuid);
    stmt.exec();
}

void VTSMessageRepository::markUnacknowledgedMessages()
{
    try
    {
        SQLite::Statement query(*vtsDatabase,
                                "UPDATE vts_messages SET acktype = 1 "
                                "WHERE acktype IS NULL AND sender = 'VTS' AND active = 1;");
        query.exec();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database update error ackUPNew: " << e.what() << std::endl;
    }
}

void VTSMessageRepository::acknowledgeThread(int threadNumber)
{
    try
    {
        SQLite::Statement query(*vtsDatabase,
                                "UPDATE vts_messages SET acktype = 2 "
                                "WHERE thread_number = ? AND sender = 'VTS' "
                                "AND (acktype IS NULL OR acktype = 1);");
        query.bind(1, threadNumber);
        query.exec();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database update error ackThread: " << e.what() << std::endl;
    }
}

std::vector<VTSMessageRepository::DBMessageRow> VTSMessageRepository::getPendingAcknowledgments(int threadNumber)
{

    std::vector<VTSMessageRepository::DBMessageRow> results;

    try
    {
        SQLite::Statement query(*vtsDatabase,
                                "SELECT uuid, ref_uuid, thread_number, filename FROM vts_messages "
                                "WHERE thread_number = ? AND sender = 'VTS' AND (acktype IS NULL OR acktype = 1);");

        query.bind(1, threadNumber);

        while (query.executeStep())
        {
            DBMessageRow row;
            row.uuid = query.getColumn(0).getString();
            row.refUUID = query.getColumn(1).isNull() ? "" : query.getColumn(1).getString();
            row.threadNumber = query.getColumn(2).getInt();
            row.filename = query.getColumn(3).getString();
            results.push_back(std::move(row));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database query error: " << e.what() << std::endl;
    }

    return results;
}

std::vector<int> VTSMessageRepository::getActiveThreadIDs()
{
    std::vector<int> threads;
    try
    {
        SQLite::Statement query(*vtsDatabase,
                                "SELECT thread_number FROM subscriptions "
                                "WHERE active = 1;");

        while (query.executeStep())
        {
            threads.push_back(query.getColumn(0).getInt());
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database query error: " << e.what() << std::endl;
    }

    return threads;
}

bool VTSMessageRepository::messageNeedsAcknowledgment(const std::string &uuid)
{
    try
    {
        SQLite::Statement query(*vtsDatabase,
                                "SELECT acktype FROM vts_messages WHERE uuid = ? AND sender = 'VTS';");

        query.bind(1, uuid);

        if (query.executeStep())
        {
            int ackType = query.getColumn(0).getInt();
            return ackType < 2; // Return true if acktype is NULL or 1
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database query error: " << e.what() << std::endl;
    }
    return false; // Default to false if something goes wrong
}

void VTSMessageRepository::acknowledgeMessage(const std::string &uuid, int acktype)
{
    try
    {
        SQLite::Statement query(*vtsDatabase,
                                "UPDATE vts_messages SET acktype = ? WHERE uuid = ? AND sender = 'VTS';");
        query.bind(1, acktype);
        query.bind(2, uuid);
        query.exec();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database update error ack message: " << e.what() << std::endl;
    }
}

bool VTSMessageRepository::messageInDatabase(const std::string &uuid)
{
    SQLite::Statement query(*vtsDatabase, "SELECT 1 FROM vts_messages WHERE uuid = ? LIMIT 1;");
    query.bind(1, uuid);
    return query.executeStep();
};

void VTSMessageRepository::insertMessage(VTSMessageRepository::DBMessageRow message)
{
    SQLite::Statement insert(*vtsDatabase,
                             "INSERT INTO vts_messages (uuid, ref_uuid, thread_number, sender, filename) VALUES (?, ?, ?, ?, ?);");
    insert.bind(1, message.uuid);
    insert.bind(2, message.refUUID.empty() ? nullptr : message.refUUID.c_str());
    insert.bind(3, message.threadNumber); // Initialize thread_number
    insert.bind(4, message.sender);
    insert.bind(5, message.filename);
    insert.exec();
}

void VTSMessageRepository::insertSubscription(std::string subid, int threadnumber)
{
    SQLite::Statement subInsert(*vtsDatabase, "INSERT INTO subscriptions (uuid_sub, thread_number, active) VALUES (?, ?, ?) "
                                              "ON CONFLICT(uuid_sub) DO UPDATE SET active = 1;");
    subInsert.bind(1, subid);
    subInsert.bind(2, threadnumber);
    subInsert.bind(3, 1);
    subInsert.exec();
}

int VTSMessageRepository::highestThreadNumber()
{
    SQLite::Statement maxQuery(*vtsDatabase, "SELECT MAX(thread_number) FROM vts_messages;");
    if (maxQuery.executeStep() && !maxQuery.getColumn(0).isNull())
    {
        return maxQuery.getColumn(0).getInt();
    }
    return 0;
}

std::vector<VTSMessageRepository::DBMessageRow> VTSMessageRepository::getUnthreadedMessages()
{

    std::vector<VTSMessageRepository::DBMessageRow> results;

    try
    {
        SQLite::Statement query(*vtsDatabase, "SELECT uuid, ref_uuid FROM vts_messages WHERE thread_number = -1;");

        while (query.executeStep())
        {
            DBMessageRow row;
            row.uuid = query.getColumn(0).getString();
            row.refUUID = query.getColumn(1).isNull() ? "" : query.getColumn(1).getString();
            row.threadNumber = query.getColumn(2).getInt();
            row.filename = query.getColumn(3).getString();
            results.push_back(std::move(row));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database query error: " << e.what() << std::endl;
    }

    return results;
}

int VTSMessageRepository::getThreadNumberOfRefID(const std::string &refid)
{
    SQLite::Statement fetchThreadNumber(*vtsDatabase,
                                        "SELECT thread_number FROM vts_messages WHERE uuid = ?;");
    fetchThreadNumber.bind(1, refid);

    if (fetchThreadNumber.executeStep() && !fetchThreadNumber.getColumn(0).isNull())
    {
        return fetchThreadNumber.getColumn(0).getInt();
    }

    return -1;
}

void VTSMessageRepository::linkThreadNumbersByRefID()
{
    // Step 1: Load all UUID → thread_number mappings
    std::unordered_map<std::string, int> uuidToThread;

    SQLite::Statement fetchAll(*vtsDatabase,
                               "SELECT uuid, thread_number FROM vts_messages WHERE thread_number IS NOT NULL");

    while (fetchAll.executeStep())
    {
        std::string uuid = fetchAll.getColumn(0).getString();
        int threadNumber = fetchAll.getColumn(1).getInt();
        uuidToThread[uuid] = threadNumber;
    }

    // Step 2: Pick rows that have non-null refid and (thread_number IS NULL or -1)
    SQLite::Statement fetchRefRows(*vtsDatabase,
                                   "SELECT uuid, ref_uuid FROM vts_messages "
                                   "WHERE ref_uuid IS NOT NULL AND (thread_number IS NULL OR thread_number = -1)");

    while (fetchRefRows.executeStep())
    {
        std::string uuid = fetchRefRows.getColumn(0).getString();
        std::string refid = fetchRefRows.getColumn(1).getString();

        // Step 3: Compare refid with all UUIDs (via map)
        auto it = uuidToThread.find(refid);
        if (it != uuidToThread.end())
        {
            int parentThread = it->second;

            // Step 4: Update thread_number of the current row
            SQLite::Statement update(*vtsDatabase,
                                     "UPDATE vts_messages SET thread_number = ? WHERE uuid = ?");
            update.bind(1, parentThread);
            update.bind(2, uuid);
            update.exec();

            // Add this UUID to the map so its children can use it
            uuidToThread[uuid] = parentThread;
        }
        // else: refid doesn't match anything — skip it
    }
}

void VTSMessageRepository::updateActiveFlagsFromSubscriptions()
{
    try
    {

        SQLite::Statement resetActive(*vtsDatabase, "UPDATE vts_messages SET active = 0;");
        resetActive.exec();

        SQLite::Statement subQuery(*vtsDatabase, "SELECT thread_number FROM subscriptions WHERE active = 1;");
        std::unordered_set<int> activeThreads;
        while (subQuery.executeStep())
        {
            if (!subQuery.getColumn(0).isNull())
            {
                activeThreads.insert(subQuery.getColumn(0).getInt());
            }
        }

        SQLite::Statement updateActive(*vtsDatabase, "UPDATE vts_messages SET active = 1 WHERE thread_number = ?;");
        for (int thread : activeThreads)
        {
            updateActive.bind(1, thread);
            updateActive.exec();
            updateActive.reset();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Active update error: " << e.what() << std::endl;
    }
}

void VTSMessageRepository::closeThread(int threadID)
{
    try
    {
        // Deactivate subscriptions
        SQLite::Statement updateSub(*vtsDatabase,
                                    "UPDATE subscriptions SET active = 0 WHERE thread_number = ? AND active = 1;");
        updateSub.bind(1, threadID);
        updateSub.exec();

        // Deactivate messages
        SQLite::Statement updateMessages(*vtsDatabase,
                                         "UPDATE vts_messages SET active = 0 WHERE thread_number = ?;");
        updateMessages.bind(1, threadID);
        updateMessages.exec();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error closing thread: " << e.what() << std::endl;
    }
}

std::vector<VTSMessageRepository::DBMessageRow> VTSMessageRepository::getMessagesOfThread(int threadNumber)
{
    std::vector<VTSMessageRepository::DBMessageRow> results;

    try
    {
        SQLite::Statement query(*vtsDatabase, "SELECT uuid, ref_uuid, thread_number, filename FROM vts_messages WHERE thread_number = ?;");
        query.bind(1, threadNumber);
        while (query.executeStep())
        {
            DBMessageRow row;
            row.uuid = query.getColumn(0).getString();
            row.refUUID = query.getColumn(1).isNull() ? "" : query.getColumn(1).getString();
            row.threadNumber = query.getColumn(2).getInt();
            row.filename = query.getColumn(3).getString();
            results.push_back(std::move(row));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database query error: " << e.what() << std::endl;
    }

    return results;
};

std::vector<VTSMessageRepository::DBMessageRow> VTSMessageRepository::getMessages()
{
    std::vector<VTSMessageRepository::DBMessageRow> results;

    try
    {
        SQLite::Statement query(*vtsDatabase, "SELECT uuid, ref_uuid, thread_number, filename FROM vts_messages;");
        while (query.executeStep())
        {
            DBMessageRow row;
            row.uuid = query.getColumn(0).getString();
            row.refUUID = query.getColumn(1).isNull() ? "" : query.getColumn(1).getString();
            row.threadNumber = query.getColumn(2).getInt();
            row.filename = query.getColumn(3).getString();
            results.push_back(std::move(row));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database query error: " << e.what() << std::endl;
    }

    return results;
};

std::string VTSMessageRepository::getSubscriptionID(int threadNumber)
{
    // Fetch the subscription UUID for this thread
    SQLite::Statement query(*vtsDatabase, "SELECT uuid_sub FROM subscriptions WHERE thread_number = ? AND active = 1;");
    query.bind(1, threadNumber);

    std::string subscriptionUUID;
    if (query.executeStep())
    {
        subscriptionUUID = query.getColumn(0).getString();
    }
    else
    {
        qWarning() << "No active subscription found for thread:" << threadNumber;
        return ""; // Nothing to unsubscribe from
    }
    return subscriptionUUID;
};

std::string VTSMessageRepository::getLatestMessageInThread(int threadNumber)
{

    // Query for the latest message in the thread (most recent by reported time)
    SQLite::Statement query(*vtsDatabase, "SELECT uuid FROM vts_messages WHERE thread_number = ? ORDER BY rowid DESC LIMIT 1;");
    query.bind(1, threadNumber);
    qDebug() << "query get ";

    if (query.executeStep())
    {
        return query.getColumn(0).getString();
    }
    return "";
};

std::string VTSMessageRepository::getFilePath(const std::string &uuid)
{
    SQLite::Statement query(*vtsDatabase, "SELECT filename FROM vts_messages WHERE uuid = ?;");

    query.bind(1, uuid);

    if (query.executeStep() && !query.getColumn(0).isNull())
    {
        return query.getColumn(0).getString();
    }
    return "";
}