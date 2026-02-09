#pragma once

#include <string>
#include <algorithm>
#include <stdexcept>

enum class messageStatusCode
{
    Requested = 1,
    Required,
    Confirmed,
    Denied,
    Cancelled
};


/*! 
 * \class MessageStatus
 * \brief Represents an S-212 message type.
 *
 * Provides conversion from int or string to enum representation,
 * and access to a human-readable string.
 */
class MessageStatus
{
private:
    messageStatusCode messageStatusValue;
    static const char *const messageStatusLiterals[5];

public:
    /*!
     * \brief Constructor from integer code.
     */
    MessageStatus(int);

     /*!
     * \brief Constructor from string representation.
     */
    MessageStatus(const std::string &);

     /*!
     * \brief Returns the human readable literal
     */
    std::string getMessageStatusLiteral();
};
