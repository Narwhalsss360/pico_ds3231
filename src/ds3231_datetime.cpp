#include <pico_ds3231/ds3231_datetime.hpp>
#include <cstring>

using namespace ds3231;

DateTime::DateTime(const tm& time)
    : time(time) {}

DateTime::DateTime(int year, int month, int day, int hour, int min, int sec, int wday)
    : DateTime()
{
    std::memset(&time, 0, sizeof(std::tm));
    time.tm_year = year - 1900;
    time.tm_mon  = month - 1;
    time.tm_mday = day;
    time.tm_hour = hour;
    time.tm_min  = min;
    time.tm_sec  = sec;
    time.tm_isdst = -1;

    if (wday >= 0) {
        time.tm_wday = wday;
    } else {
        std::tm temp = time;
        if (std::mktime(&temp) != static_cast<std::time_t>(-1)) {
            time = temp;
        }
    }
}

DateTime::DateTime(std::time_t epoch)
    : DateTime()
{
    std::memset(&time, 0, sizeof(std::tm));
    if (std::tm* local_ptr = std::localtime(&epoch)) {
        time = *local_ptr;
    }
}

DateTime::DateTime(const ::datetime_t& pico_dt)
    : DateTime() {
        std::memset(&time, 0, sizeof(std::tm));
        time.tm_year = pico_dt.year - 1900;
        time.tm_mon  = pico_dt.month - 1;
        time.tm_mday = pico_dt.day;
        time.tm_hour = pico_dt.hour;
        time.tm_min  = pico_dt.min;
        time.tm_sec  = pico_dt.sec;
        time.tm_wday = pico_dt.dotw;
        time.tm_isdst = -1;
}

std::time_t DateTime::to_epoch() const {
    std::tm temp = time;
    return std::mktime(&temp);
}

::datetime_t DateTime::to_pico_datetime() const {
    ::datetime_t pico_dt;
    pico_dt.year  = static_cast<int16_t>(time.tm_year + 1900);
    pico_dt.month = static_cast<int8_t>(time.tm_mon + 1);
    pico_dt.day   = static_cast<int8_t>(time.tm_mday);
    pico_dt.hour  = static_cast<int8_t>(time.tm_hour);
    pico_dt.min   = static_cast<int8_t>(time.tm_min);
    pico_dt.sec   = static_cast<int8_t>(time.tm_sec);

    if (time.tm_wday >= 0 && time.tm_wday <= 6) {
        pico_dt.dotw = static_cast<int8_t>(time.tm_wday);
    } else {
        std::tm temp = time;
        if (std::mktime(&temp) != static_cast<std::time_t>(-1)) {
            pico_dt.dotw = static_cast<int8_t>(temp.tm_wday);
        } else {
            pico_dt.dotw = 0;
        }
    }
    return pico_dt;
}


DateTime ds3231::subtract_calendar(const DateTime& lhs, const DateTime& rhs) {
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
