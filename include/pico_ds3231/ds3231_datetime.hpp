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

    class DateTime {
    public:
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

    class TimeSpan {
    private:
        std::int64_t _seconds;

    public:
        constexpr TimeSpan() : _seconds(0) {}

        constexpr TimeSpan(std::int64_t seconds) : _seconds(seconds) {}

        constexpr TimeSpan(int days, int hours, int minutes, int seconds)
            : _seconds((static_cast<std::int64_t>(days) * 86400) +
                       (static_cast<std::int64_t>(hours) * 3600) +
                       (static_cast<std::int64_t>(minutes) * 60) +
                       seconds) {}

        constexpr std::int64_t total_seconds() const { return _seconds; }

        constexpr int days() const    { return static_cast<int>(_seconds / 86400); }

        constexpr int hours() const   { return static_cast<int>((_seconds % 86400) / 3600); }

        constexpr int minutes() const { return static_cast<int>((_seconds % 3600) / 60); }

        constexpr int seconds() const { return static_cast<int>(_seconds % 60); }

        constexpr TimeSpan operator+(const TimeSpan& rhs) const { return TimeSpan(_seconds + rhs._seconds); }

        constexpr TimeSpan operator-(const TimeSpan& rhs) const { return TimeSpan(_seconds - rhs._seconds); }

        constexpr bool operator==(const TimeSpan& rhs) const { return _seconds == rhs._seconds; }

        constexpr bool operator!=(const TimeSpan& rhs) const { return _seconds != rhs._seconds; }

        constexpr bool operator<(const TimeSpan& rhs)  const { return _seconds < rhs._seconds; }

        constexpr bool operator<=(const TimeSpan& rhs) const { return _seconds <= rhs._seconds; }

        constexpr bool operator>(const TimeSpan& rhs)  const { return _seconds > rhs._seconds; }

        constexpr bool operator>=(const TimeSpan& rhs) const { return _seconds >= rhs._seconds; }
    };

    namespace operators {
        inline TimeSpan operator-(const DateTime& lhs, const DateTime& rhs) {
            return TimeSpan(static_cast<std::int64_t>(lhs.to_epoch() - rhs.to_epoch()));
        }

        inline DateTime operator+(const DateTime& lhs, const TimeSpan& rhs) {
            return DateTime(lhs.to_epoch() + rhs.total_seconds());
        }

        inline DateTime operator-(const DateTime& lhs, const TimeSpan& rhs) {
            return DateTime(lhs.to_epoch() - rhs.total_seconds());
        }
    }

    using namespace operators;

    DateTime subtract_calendar(const DateTime& lhs, const DateTime& rhs);
}
