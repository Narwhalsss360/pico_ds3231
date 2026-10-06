#pragma once
#include <ctime>
#include <hardware/rtc.h>

namespace ds3231 {
    constexpr const char compilation_months[12][4] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };

    class DateTime {
    public:
        tm time;

        DateTime();

        DateTime(const tm& time);

        DateTime(int year, int month, int day, int hour = 0, int min = 0, int sec = 0, int wday = -1);

        DateTime(std::time_t epoch);

        DateTime(const ::datetime_t& pico_dt);

        std::time_t to_epoch() const;

        ::datetime_t to_pico_datetime() const;
    };
}
