#include "subjectOfCommunication.hpp"

SubjectOfCommunication::SubjectOfCommunication(int v)
{
    subjectOfCommunicationValue = subjectOfCommunicationCode(v);
}

SubjectOfCommunication::SubjectOfCommunication(const std::string &subject)
{
    // Find the subject string in the literals list
    auto it = std::find(std::begin(subjectOfCommunicationLiterals), std::end(subjectOfCommunicationLiterals), subject);

    // If found, assign the corresponding enum value
    if (it != std::end(subjectOfCommunicationLiterals))
    {
        subjectOfCommunicationValue = subjectOfCommunicationCode(std::distance(std::begin(subjectOfCommunicationLiterals), it));
    }
    else
    {
        // Throw an exception if the subject string is invalid
        throw std::invalid_argument("Invalid subject of communication string: " + subject);
    }
}

std::string SubjectOfCommunication::getSubjectOfCommunicationLiteral()
{
    int labelIndex = static_cast<int>(subjectOfCommunicationValue);

    return subjectOfCommunicationLiterals[labelIndex];
}
const char *const SubjectOfCommunication::subjectOfCommunicationLiterals[11] =
    {
        "Arrival Anchoring Operation",
        "Arrival Berth",
        "Arrival Portarea",
        "Arrival Vtsarea",
        "Departure Anchoring Operation",
        "Departure Berth",
        "Departure Portarea",
        "Departure Vtsarea",
        "Pilotage",
        "Port Visit",
        "Ready to Sail Operation"};
