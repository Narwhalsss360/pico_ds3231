#pragma once
#include "ds3231_datetime.hpp"

namespace ds3231 {
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

}
