// Date.hpp

#ifndef DATE_HPP
#define DATE_HPP

#include <string>
#include <iostream>

class Date
{
public:
    int year, month, day;


    Date() {};
    Date(int y, int m, int d) : year(y), month(m), day(d) {}

    // Constructor to parse a string in the format %Y-%m-%d or %Y%m%d
    Date(const std::string &dateStr)
    {
        if (dateStr.size() == 8)
        {
            year = std::stoi(dateStr.substr(0, 4));
            month = std::stoi(dateStr.substr(4, 2));
            day = std::stoi(dateStr.substr(6, 2));
        }
        else if (dateStr.size() == 10)
        {
            year = std::stoi(dateStr.substr(0, 4));
            month = std::stoi(dateStr.substr(5, 2));
            day = std::stoi(dateStr.substr(8, 2));
        }
        else
        {
            throw std::invalid_argument("Invalid date format");
        }
    }
    // Helper function to zero-pad single-digit numbers
    std::string zeroPad(int num) const
    {
        return (num < 10) ? "0" + std::to_string(num) : std::to_string(num);
    }
    // format can be ISO or XML
    std::string toString(std::string format = "XML") const
    {
        if (format == "ISO")
        {
            return std::to_string(year) + zeroPad(month) + zeroPad(day);
        }
        else if (format == "XML")
        {
            return std::to_string(year) + '-' + zeroPad(month) + '-' + zeroPad(day);
        }
        else
        {
            return "";
        }
    }

    // Friend comparison operators
    friend bool operator==(const Date &lhs, const Date &rhs);
    friend bool operator<(const Date &lhs, const Date &rhs);

    // Additional comparison operators
    friend bool operator!=(const Date &lhs, const Date &rhs);
    friend bool operator<=(const Date &lhs, const Date &rhs);
    friend bool operator>(const Date &lhs, const Date &rhs);
    friend bool operator>=(const Date &lhs, const Date &rhs);
};

// Comparison operator implementations (also inside the header)
inline bool operator==(const Date &lhs, const Date &rhs)
{
    return lhs.day == rhs.day && lhs.month == rhs.month && lhs.year == rhs.year;
}

inline bool operator<(const Date &lhs, const Date &rhs)
{
    if (lhs.year != rhs.year)
        return lhs.year < rhs.year;
    if (lhs.month != rhs.month)
        return lhs.month < rhs.month;
    return lhs.day < rhs.day;
}

inline bool operator!=(const Date &lhs, const Date &rhs) { return !(lhs == rhs); }
inline bool operator<=(const Date &lhs, const Date &rhs) { return !(rhs < lhs); }
inline bool operator>(const Date &lhs, const Date &rhs) { return rhs < lhs; }
inline bool operator>=(const Date &lhs, const Date &rhs) { return !(lhs < rhs); }

#endif // DATE_HPP
