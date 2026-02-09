// DateTime.hpp

#ifndef DATE_TIME_HPP
#define DATE_TIME_HPP

#include <string>
#include <iostream>
#include <chrono>
#include "Date.hpp"
#include "Time.hpp"

class DateTime
{
public:
    Date date;
    Time time;

    DateTime(const std::string &dateTimeStr)
    {
        size_t tPos = dateTimeStr.find("T");
        if (tPos == std::string::npos)
        {
            throw std::invalid_argument("Invalid DateTime format");
            return;
        }

        size_t dashPos = dateTimeStr.find("-");
        if (dashPos == std::string::npos)
        {
            throw std::invalid_argument("Invalid DateTime format");
            return;
        }

        std::string dateString = dateTimeStr.substr(0, tPos);
        std::string timeString = dateTimeStr.substr(tPos + 1);

        date = Date(dateString);
        time = Time(timeString);
    }

    DateTime(int year, int month, int day, int hour, int min, int sec)
    {
        date = Date(year, month, day);
        time = Time(hour, min, sec, true);
    }

    DateTime(int year, int month, int day, int hour, int min, int sec, int offset, bool utc)
    {
        date = Date(year, month, day);
        time = Time(hour, min, sec, offset, utc);
    }

    DateTime()
    {
        using namespace std::chrono;


    auto now = system_clock::now();
    std::time_t now_c = system_clock::to_time_t(now);

    std::tm utc_tm{};  // zero-init

#ifdef _WIN32
    gmtime_s(&utc_tm, &now_c);
#else
    gmtime_r(&now_c, &utc_tm);
#endif
        // Create DateTime object
        date = Date(utc_tm.tm_year + 1900, utc_tm.tm_mon + 1, utc_tm.tm_mday);
        time = Time(utc_tm.tm_hour, utc_tm.tm_min, utc_tm.tm_sec, true);
    }
    // format can be ISO or XML
    std::string toString(std::string format = "XML", bool addZ = true) const
    {
        return date.toString(format) + "T" + time.toString(format, 0, addZ);
    }

    // format can be ISO or XML
    std::string toTimeString(std::string format = "XML", int offsetter = 0) const
    {
        return time.toString(format, offsetter);
    }

    // format can be ISO or XML
    std::string toDateString(std::string format = "XML") const
    {
        return date.toString(format);
    }

    // Comparison operators
    friend bool operator==(const DateTime &lhs, const DateTime &rhs);
    friend bool operator<(const DateTime &lhs, const DateTime &rhs);
    // Additional operators derived from above
    friend bool operator!=(const DateTime &lhs, const DateTime &rhs);
    friend bool operator<=(const DateTime &lhs, const DateTime &rhs);
    friend bool operator>(const DateTime &lhs, const DateTime &rhs);
    friend bool operator>=(const DateTime &lhs, const DateTime &rhs);
};

// Comparison operator implementations (inside the header, inline)
inline bool operator==(const DateTime &lhs, const DateTime &rhs)
{
    return lhs.date == rhs.date && lhs.time == rhs.time;
}

inline bool operator<(const DateTime &lhs, const DateTime &rhs)
{
    if (lhs.date != rhs.date)
        return lhs.date < rhs.date;
    return lhs.time < rhs.time;
}

inline bool operator!=(const DateTime &lhs, const DateTime &rhs) { return !(lhs == rhs); }
inline bool operator<=(const DateTime &lhs, const DateTime &rhs) { return !(rhs < lhs); }
inline bool operator>(const DateTime &lhs, const DateTime &rhs) { return rhs < lhs; }
inline bool operator>=(const DateTime &lhs, const DateTime &rhs) { return !(lhs < rhs); }

#endif // DATE_TIME_HPP
