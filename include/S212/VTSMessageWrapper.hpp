// VTSMessageWrapper.hpp

#pragma once

#include "VTSMessageWidget.hpp"
#include <QListWidgetItem>
#include "VTSDigitalInformationMessage.hpp"
#include "pugixml.hpp"
#include <random>

enum Sender
{
    Ship,
    VTS
};
enum ackType
{
    Delivered = 1,
    Opened,
    Error
};

/*!
 * \class VTSMessageWrapper
 * \brief UI-aware wrapper around a VTS digital message.
 *
 * Handles UUID generation, XML export, item decoration for QListWidget,
 * and access to key VTS metadata fields (e.g. subject, effective time).
 */
class VTSMessageWrapper
{
public:
    /*!
     * \brief Construct a wrapper around a VTS message.
     * \param message Pointer to parsed digital message (ownership taken).
     * \param sender Indicates whether message is from VTS or Ship.
     */
    VTSMessageWrapper(VTSDigitalInformationMessage *message, Sender sender);

    ~VTSMessageWrapper();

    /*!
     * \brief Returns the decorated list item for display in QListWidget.
     */
    QListWidgetItem *getQListWidgetItem();

    /*!
     * \brief Serializes the message to a valid S-212 XML file on disk.
     *
     * Builds an XML document conforming to S-212 and writes it to
     * a file named `filename` in the `../data/VTS/` directory.
     */
    void writeToFile();

    /*!
     * \brief Generates a random UUIDv4 and sets filename accordingly.
     *
     * UUID is assigned in the 8-4-4-4-12 format.
     */
    void generateUUID();

    /*!
     * \brief Returns reference message ID if present, empty otherwise.
     */
    std::string getReferenceId();

    /*!
     * \brief Returns the port call ID if present.
     */
    std::string getPortCallID();

    /*!
     * \brief Returns the message subject (as subjectOfCommunication literal).
     */
    std::string getSubject();

    /*!
     * \brief Returns the first clearance location URN.
     */
    std::string getLocation();

    /*!
     * \brief Returns the message's effective time.
     */
    DateTime getEffectiveTime();

    /*!
     * \brief Returns the message report timestamp.
     */
    DateTime getReportTime();

    /*!
     * \brief Returns the optional windowAfter value or "00:00" fallback.
     */
    std::string getWindowAfter();

    /*!
     * \brief Validates the XML output file using xmllint and S-212 schema.
     * \param filePath Path to the XML file to validate.
     */
    void validateXML(const std::string &filePath);

    std::string filename; ///< Output XML file name (derived from UUID)
    std::string uuid;     ///< UUID for this message
    ackType acknowledged; ///< Acknowledgment status
    int threadNumber;     ///< Associated thread ID
    VTSMessageWidget *vtsMessage = nullptr; ///< UI widget for rendering

private:
    /*!
     * \brief Generates the QListWidgetItem used for display.
     *
     * Sets a fixed height and stores the message pointer in Qt's user role.
     */
    QListWidgetItem *generateQListWidgetItem();

    /*!
     * \brief Decorates the UI item with sender and display field content.
     *
     * Creates a new VTSMessageWidget based on sender and display fields.
     */
    void decorateItem();

    Sender sender; ///< Message sender type (VTS or Ship)
    VTSDigitalInformationMessage *message; ///< Raw parsed message (owned)
    QListWidgetItem *listItem = nullptr; ///< Cached item for QListWidget
};
