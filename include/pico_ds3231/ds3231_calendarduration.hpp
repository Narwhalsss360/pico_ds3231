#pragma once
#include "ds3231_timespan.hpp"

namespace ds3231 {
    class CalendarDuration {
    private:
        struct Result;

    public:
        inline CalendarDuration();

        inline CalendarDuration(const DateTime& from, const DateTime& to);

        inline DateTime from() const {
            return _from;
        }

        inline DateTime to() const {
            return _to;
        }

        inline int years() const {
            return result.years;
        }

        inline int months() const {
            return result.months;
        }

        inline int days() const {
            return result.days;
        }

        inline int hours() const {
            return result.hours;
        }

        inline int minutes() const {
            return result.minutes;
        }

        inline int seconds() const {
            return result.seconds;
        }

        inline bool valid() const {
            return _from.valid() && _to.valid();
        }

        inline operator bool() const {
            return valid();
        }

        inline operator TimeSpan() const {
            return _to - _from;
        }

    private:
        DateTime _from, _to;

        struct Result {
            int years = 0;
            int months = 0;
            int days = 0;
            int hours = 0;
            int minutes = 0;
            int seconds = 0;
        } result;

        static inline Result subtract_calendar(const DateTime& lhs, const DateTime& rhs);
    };


    inline CalendarDuration::Result CalendarDuration::subtract_calendar(const DateTime& lhs, const DateTime& rhs) {
        const auto& a = lhs.time;
        const auto& b = rhs.time;

        const bool lhs_is_earlier = (
            (a.tm_year < b.tm_year) ||
            (a.tm_year == b.tm_year && a.tm_mon < b.tm_mon) ||
            (a.tm_year == b.tm_year && a.tm_mon == b.tm_mon && a.tm_mday < b.tm_mday) ||
            (a.tm_year == b.tm_year && a.tm_mon == b.tm_mon && a.tm_mday == b.tm_mday && a.tm_hour < b.tm_hour) ||
            (a.tm_year == b.tm_year && a.tm_mon == b.tm_mon && a.tm_mday == b.tm_mday && a.tm_hour == b.tm_hour && a.tm_min < b.tm_min) ||
            (a.tm_year == b.tm_year && a.tm_mon == b.tm_mon && a.tm_mday == b.tm_mday && a.tm_hour == b.tm_hour && a.tm_min == b.tm_min && a.tm_sec < b.tm_sec)
        );

        if (lhs_is_earlier) {
            return {};
        }

        Result diff {
            .years = a.tm_year - b.tm_year,
            .months = a.tm_mon  - b.tm_mon,
            .days = a.tm_mday - b.tm_mday,
            .hours = a.tm_hour - b.tm_hour,
            .minutes = a.tm_min  - b.tm_min,
            .seconds = a.tm_sec  - b.tm_sec
        };

        if (diff.seconds < 0) {
            diff.seconds += 60;
            --diff.minutes;
        }

        if (diff.minutes < 0) {
            diff.minutes += 60;
            --diff.hours;
        }

        if (diff.hours < 0) {
            diff.hours += 24;
            --diff.days;
        }

        if (diff.days < 0) {
            int prev_mon = a.tm_mon - 1;
            int prev_year = a.tm_year;

            if (prev_mon < 0) {
                prev_mon = 11;
                --prev_year;
            }

            diff.days += days_in_month(prev_mon, prev_year);
            --diff.months;
        }

        if (diff.months < 0) {
            diff.months += 12;
            --diff.years;
        }

        return diff;
    }

    inline CalendarDuration::CalendarDuration()
        : _from(invalid_time), _to(invalid_time), result{} {}

    inline CalendarDuration::CalendarDuration(const DateTime& from, const DateTime& to)
        : _from(from), _to(to), result(subtract_calendar(to, from)) {}
}
