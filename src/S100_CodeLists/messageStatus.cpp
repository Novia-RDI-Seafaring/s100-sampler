#include "messageStatus.hpp"

MessageStatus::MessageStatus(int v)
{
    messageStatusValue = messageStatusCode(v);
}

MessageStatus::MessageStatus(const std::string &status)
{
    // Find the status string in the literals list
    auto it = std::find(std::begin(messageStatusLiterals), std::end(messageStatusLiterals), status);

    // If found, assign the corresponding enum value
    if (it != std::end(messageStatusLiterals))
    {
        messageStatusValue = messageStatusCode(std::distance(std::begin(messageStatusLiterals), it));
    }
    else
    {
        // Throw an exception if the status string is invalid
        throw std::invalid_argument("Invalid message status string: " + status);
    }
}

std::string MessageStatus::getMessageStatusLiteral()
{
    int labelIndex = static_cast<int>(messageStatusValue);

    return messageStatusLiterals[labelIndex];
}
const char *const MessageStatus::messageStatusLiterals[5] =
    {
        "Requested",
        "Required",
        "Confirmed",
        "Denied",
        "Cancelled"};
