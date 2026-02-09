// TruncatedDate.hpp

#ifndef TRUNCATED_DATE_HPP
#define TRUNCATED_DATE_HPP

#include <string>
#include <iostream>
#include "Date.hpp"

class TruncatedDate : public Date
{
private:
    int year = -1, month = -1, day = -1;

public:
    TruncatedDate(){};
    TruncatedDate(int y, int m, int d) : year(y), month(m), day(d) {}

    // 1992-12-04 [10]
    //--04-23 [7]
    //--12-- [6]
    //----12 [6]

    TruncatedDate(const std::string &dateStr)
    {
        if (dateStr.size() == 10)
        {
            year = std::stoi(dateStr.substr(0, 4));
            month = std::stoi(dateStr.substr(5, 2));
            day = std::stoi(dateStr.substr(7, 2));
            return;
        }
        if (dateStr.size() == 7)
        {
            month = std::stoi(dateStr.substr(2, 2));
            day = std::stoi(dateStr.substr(5, 2));
            return;
        }
        if (dateStr.size() == 6 && dateStr[dateStr.size() - 1] != '-')
        {
            day = std::stoi(dateStr.substr(4, 2));
            return;
        }
        if (dateStr.size() == 6 && dateStr[dateStr.size() - 1] == '-')
        {
            month = std::stoi(dateStr.substr(2, 2));
            return;
        }
        else
        {
            throw std::invalid_argument("Invalid date format");
        }
    }

    std::string toString() const
    {
        std::string yearString, monthString, dayString;

        auto zeroPadOrDash = [this](int value)
        {
            return value == -1 ? "-" : zeroPad(value);
        };

        return zeroPadOrDash(year) + '-' + zeroPadOrDash(month) + '-' + zeroPadOrDash(day);
    }
};

#endif // TRUNCATED_DATE_HPP
