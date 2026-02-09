#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>

enum class subjectOfCommunicationCode
{
    Arrival_Anchoring_Operation = 1,
    Arrival_Berth,
    Arrival_Portarea,
    Arrival_Vtsarea,
    Departure_Anchoring_Operation,
    Departure_Berth,
    Departure_Portarea,
    Departure_Vtsarea,
    Pilotage,   // Arrival type
    Port_Visit, // Arrival type
    Ready_To_Sail_Operation
};

/*!
 * \class SubjectOfCommunication
 * \brief Represents an S-212 communication subject, i.e. the type of VTS event
 *
 * Provides conversion from int or string to enum representation,
 * and access to a human-readable string.
 */
class SubjectOfCommunication
{
private:
    subjectOfCommunicationCode subjectOfCommunicationValue;
    static const char *const subjectOfCommunicationLiterals[11];

public:
    /*!
     * \brief Constructor from integer code.
     */
    SubjectOfCommunication(int);
    /*!
     * \brief Constructor from string representation.
     */
    SubjectOfCommunication(const std::string &);
    /*!
     * \brief Returns the human readable literal
     */
    std::string getSubjectOfCommunicationLiteral();
};