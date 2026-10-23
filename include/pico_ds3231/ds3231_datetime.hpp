#pragma once
#include <ctime>
#include <cstdint>
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

    constexpr tm invalid_time {
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1
    };

    constexpr bool is_leap_year(const int& tm_year) {
        const int year = tm_year + 1900;
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    constexpr int days_in_month(const int& tm_mon, const int& tm_year) {
        constexpr int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        return (tm_mon == 1 && is_leap_year(tm_year)) ? (29) : ((tm_mon >= 0 && tm_mon <= 11) ? days[tm_mon] : 0);
    }

    constexpr bool valid(const std::tm& time) {
        return (
            0 <= time.tm_sec  && time.tm_sec  <= 60 &&
            0 <= time.tm_min  && time.tm_min  <= 59 &&
            0 <= time.tm_hour && time.tm_hour <= 23 &&
            0 <= time.tm_mon  && time.tm_mon  <= 11 &&
            1 <= time.tm_mday && time.tm_mday <= days_in_month(time.tm_mon, time.tm_year) &&
            0 <= time.tm_wday && time.tm_wday <= 6  &&
            0 <= time.tm_yday && time.tm_yday <  (is_leap_year(time.tm_year) ? 366 : 365)
        );
    }

    constexpr bool valid_date_and_time(const std::tm& time) {
        return (
            0 <= time.tm_sec  && time.tm_sec  <= 60 &&
            0 <= time.tm_min  && time.tm_min  <= 59 &&
            0 <= time.tm_hour && time.tm_hour <= 23 &&
            0 <= time.tm_mon  && time.tm_mon  <= 11 &&
            1 <= time.tm_mday && time.tm_mday <= days_in_month(time.tm_mon, time.tm_year)
        );
    }

    namespace days_of_the_week {
        using int_type = int8_t;

        enum DayOfTheWeek : int_type {
            Sunday,
            Monday,
            Tuesday,
            Wednesday,
            Thursday,
            Friday,
            Saturday,
        };

        constexpr int8_t invalid_day_of_week = -1;

        constexpr int8_t min = Sunday;

        constexpr int8_t max = Saturday;
    }

    using days_of_the_week::DayOfTheWeek;

    using days_of_the_week::invalid_day_of_week;

    class DateTime {
    public:
        struct compile_time_tag {};

        static constexpr compile_time_tag compile_time{};

        tm time;

        constexpr DateTime(const char* date, const char* time_str)
            : time({})
        {
            int extracted_month = 0;
            for (int i = 0; i < 12; ++i) {
                if (date[0] == compilation_months[i][0] &&
                    date[1] == compilation_months[i][1] &&
                    date[2] == compilation_months[i][2]) {
                    extracted_month = i;
                    break;
                }
            }
            time.tm_mon = extracted_month;

            if (date[4] == ' ') {
                time.tm_mday = date[5] - '0';
            } else {
                time.tm_mday = (date[4] - '0') * 10 + (date[5] - '0');
            }

            int full_year = (date[7] - '0') * 1000 +
                            (date[8] - '0') * 100 +
                            (date[9] - '0') * 10 +
                            (date[10] - '0');
            time.tm_year = full_year - 1900;

            time.tm_hour = (time_str[0] - '0') * 10 + (time_str[1] - '0');
            time.tm_min  = (time_str[3] - '0') * 10 + (time_str[4] - '0');
            time.tm_sec  = (time_str[6] - '0') * 10 + (time_str[7] - '0');

            time.tm_isdst = -1;
        }

        constexpr DateTime()
            : time({ .tm_year = -1 }) {}

        DateTime(const tm& time);

        explicit constexpr DateTime(int year, int month, int day, int hour, int min, int sec, days_of_the_week::int_type wday, int yearday, int isdst, compile_time_tag)
            : time({
                .tm_sec  = sec,
                .tm_min  = min,
                .tm_hour = hour,
                .tm_mday = day,
                .tm_mon  = month - 1,
                .tm_year = year - 1900,
                .tm_wday = (days_of_the_week::min <= wday && wday <= days_of_the_week::max) ? wday : invalid_day_of_week,
                .tm_yday = yearday,
                .tm_isdst = isdst
            }) {}

        static inline constexpr DateTime compiled(int year, int month, int day, int hour = 0, int min = 0, int sec = 0, int wday = -1, int yearday = 0, int isdst = -1) {
            return DateTime(year, month, day, hour, min, sec, wday, yearday, isdst, compile_time);
        }

        DateTime(int year, int month, int day, int hour = 0, int min = 0, int sec = 0, int wday = -1);

        DateTime(std::time_t epoch);

        DateTime(const ::datetime_t& pico_dt);

        std::time_t to_epoch() const;

        ::datetime_t to_pico_datetime() const;

        inline DateTime& fix_dow() {
            std::time_t corrected_epoch = std::mktime(&time);
            if (corrected_epoch == -1) {
                time = invalid_time;
            }

            return *this;
        }

        inline bool valid() const {
            return ::ds3231::valid(time);
        }

        inline operator const bool() const {
            return valid();
        }

        inline explicit operator const std::time_t() const {
            return to_epoch();
        }

        inline operator const ::datetime_t() const {
            return to_pico_datetime();
        }
    };

    constexpr DateTime compile_time = DateTime(__DATE__, __TIME__);

    constexpr DateTime epoch_start = DateTime::compiled(1970, 1, 1, 0, 0, 0, DayOfTheWeek::Thursday);

    inline static DateTime subtract_calendar(const DateTime& lhs, const DateTime& rhs) {
        if (lhs.to_epoch() < rhs.to_epoch()) {
            return DateTime();
        }

        int sec   = lhs.time.tm_sec   - rhs.time.tm_sec;
        int min   = lhs.time.tm_min   - rhs.time.tm_min;
        int hour  = lhs.time.tm_hour  - rhs.time.tm_hour;
        int day   = lhs.time.tm_mday  - rhs.time.tm_mday;
        int month = lhs.time.tm_mon   - rhs.time.tm_mon;
        int year  = lhs.time.tm_year  - rhs.time.tm_year;

        if (sec < 0) {
            sec += 60;
            min--;
        }

        if (min < 0) {
            min += 60;
            hour--;
        }

        if (hour < 0) {
            hour += 24;
            day--;
        }

        if (day < 0) {
            int prev_mon = lhs.time.tm_mon - 1;
            int prev_year = lhs.time.tm_year;
            if (prev_mon < 0) {
                prev_mon = 11;
                prev_year--;
            }

            day += days_in_month(prev_mon, prev_year);
            month--;
        }

        if (month < 0) {
            month += 12;
            year--;
        }

        return DateTime(year + 1900, month + 1, day, hour, min, sec);
    }
}
