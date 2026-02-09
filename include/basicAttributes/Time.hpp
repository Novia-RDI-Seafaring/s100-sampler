// Time.hpp

#ifndef TIME_HPP
#define TIME_HPP

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

class Time
{

public:
    int hours;
    int minutes;
    int seconds;
    bool utc = false;
    int m_offset = 0;
    std::tm utc_tm{}; // Weird requirement;
    
    Time() : hours(0), minutes(0), seconds(0)
    {
        std::time_t now_c = std::time(nullptr);
#ifdef _WIN32
        gmtime_s(&utc_tm, &now_c);
#else
        gmtime_r(&now_c, &utc_tm);
#endif
    }

    Time(int h, int m, int s, int off, bool isUtc) : hours(h), minutes(m), seconds(s), m_offset(off), utc(isUtc) {}
    Time(int h, int m, int s, bool isUtc) : hours(h), minutes(m), seconds(s), utc(isUtc) {}

    Time(const std::string &timeString)
    {
        parseTime(timeString);
    }
    // 07:38:39.000Z
    void parseTime(const std::string &timeString)
    {
        // size_t dotPos = timeString.find(".");
        size_t plusPos = timeString.find("+");
        size_t minusPos = timeString.find("-");
        size_t zPos = timeString.find("Z");

        hours = std::stoi(timeString.substr(0, 2));
        minutes = std::stoi(timeString.substr(3, 2));
        seconds = std::stoi(timeString.substr(6, 2));

        if (zPos != std::string::npos)
        {
            utc = true;
            return;
        }
        if (plusPos != std::string::npos || minusPos != std::string::npos)
        {
            size_t pos;
            int sign = 1;
            if (plusPos != std::string::npos)
            {
                pos = plusPos;
            }
            else
            {
                pos = minusPos;
                sign = -1;
            }

            m_offset = sign * std::stoi(timeString.substr(pos + 1, 2));
            return;
        }
    }
    // Obsolete
    void printTime()
    {
        std::cout << std::setw(2) << std::setfill('0') << hours << ":"
                  << std::setw(2) << std::setfill('0') << minutes << ":"
                  << std::setw(2) << std::setfill('0') << seconds;
        if (utc)
        {
            std::cout << "Z";
        }
    }
    std::string zeroPad(int num) const
    {
        return (abs(num) < 10) ? "0" + std::to_string(abs(num)) : std::to_string(abs(num));
    }
    std::string toString(std::string format = "XML", int offset = 0, bool addZ = true) const
    {
        std::string timeString; // = zeroPad(hours) + ":" + zeroPad(minutes) + ":" + zeroPad(seconds);
        std::cout << "Internal offset " << m_offset << " ext " << offset << '\n';
        int offsetHours = hours;
        if (m_offset != offset)
        {
            std::cout << "Offsets not mathcing, adding time" << '\n';
            offsetHours += offset;
        }

        if (format == "XML")
        {
            timeString = zeroPad(offsetHours) + ":" + zeroPad(minutes) + ":" + zeroPad(seconds);
        }
        else if (format == "ISO")
        {
            timeString = zeroPad(offsetHours) + zeroPad(minutes) + zeroPad(seconds);
        }
        else
        {
            std::cout << "Huh?" << '\n';
        }

        if (utc && offset == 0 && addZ)
        {
            timeString += "Z";
        }
        // else if (offset > 0)
        // {
        //     timeString += "+" + zeroPad(offset) + ":" + "00";
        // }
        // else if (offset < 0)
        // {
        //     timeString += "-" + zeroPad(offset) + ":" + "00";
        // }
        return timeString;
    }

    // Friend comparison operators
    friend bool operator==(const Time &lhs, const Time &rhs);
    friend bool operator<(const Time &lhs, const Time &rhs);

    // Additional comparison operators (using the same principles)
    friend bool operator!=(const Time &lhs, const Time &rhs);
    friend bool operator<=(const Time &lhs, const Time &rhs);
    friend bool operator>(const Time &lhs, const Time &rhs);
    friend bool operator>=(const Time &lhs, const Time &rhs);
};

// Comparison operator implementations (also inside the header)
inline bool operator==(const Time &lhs, const Time &rhs)
{
    return lhs.hours == rhs.hours && lhs.minutes == rhs.minutes && lhs.seconds == rhs.seconds;
}

inline bool operator<(const Time &lhs, const Time &rhs)
{
    if (lhs.hours - lhs.m_offset != rhs.hours - rhs.m_offset)
        return lhs.hours - lhs.m_offset < rhs.hours - rhs.m_offset;
    if (lhs.minutes != rhs.minutes)
        return lhs.minutes < rhs.minutes;
    return lhs.seconds < rhs.seconds;
}

inline bool operator!=(const Time &lhs, const Time &rhs) { return !(lhs == rhs); }
inline bool operator<=(const Time &lhs, const Time &rhs) { return !(rhs < lhs); }
inline bool operator>(const Time &lhs, const Time &rhs) { return rhs < lhs; }
inline bool operator>=(const Time &lhs, const Time &rhs) { return !(lhs < rhs); }

#endif // TIME_HPP
