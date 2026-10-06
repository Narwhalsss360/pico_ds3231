#include <pico_ds3231/ds3231_registers.hpp>

using namespace ds3231;

bool RegisterDeclaration<Register::timekeeping>::is_12_hour(const uint8_t (&buffer)[register_length]) {
    return (buffer[0x02] & 0b01000000) > 0;
}

void RegisterDeclaration<Register::timekeeping>::buffer_to_tm(const uint8_t (&buffer)[register_length], tm& time) {
    time.tm_sec = buffer[af_seconds] & 0b00001111;
    time.tm_sec += 10 * ((buffer[af_seconds] & 0b01110000) >> 4);

    time.tm_min = buffer[af_minutes] & 0b00001111;
    time.tm_min += 10 * ((buffer[af_minutes] & 0b01110000) >> 4);

    time.tm_hour = buffer[af_hours] & 0b00001111;
    if (is_12_hour(buffer)) {
        time.tm_hour += 10 * ((buffer[af_hours] & 0b00010000) >> 4);
        const bool is_pm = (buffer[af_hours] & 0b00100000) > 0;
        if (is_pm) {
            if (time.tm_hour != 12) {
                time.tm_hour += 12;
            }
        } else {
            if (time.tm_hour == 12) {
                time.tm_hour = 0;
            }
        }
    } else {
        time.tm_hour += 10 * ((buffer[af_hours] & 0b00110000) >> 4);
    }

    time.tm_mday = buffer[af_date] & 0b00001111;
    time.tm_mday += 10 * ((buffer[af_date] & 0b00110000) >> 4);

    time.tm_mon = buffer[af_month_and_century] & 0b00001111;
    time.tm_mon += 10 * ((buffer[af_month_and_century] & 0b00010000) >> 4);
    time.tm_mon -= 1;

    time.tm_year = ((buffer[af_month_and_century] & 0b10000000) > 0) ? 100 : 0;
    time.tm_year += buffer[af_year] & 0b00001111;
    time.tm_year += 10 * ((buffer[af_year] & 0b11110000) >> 4);

    time.tm_wday = (buffer[af_day] & 0b00000111);
    time.tm_wday -= 1;
}

tm RegisterDeclaration<Register::timekeeping>::buffer_to_tm(const uint8_t (&buffer)[register_length]) {
    tm time;
    buffer_to_tm(buffer, time);
    return time;
}

void RegisterDeclaration<Register::timekeeping>::tm_to_buffer(const tm& time, uint8_t (&buffer)[register_length]) {
    buffer[af_seconds] = static_cast<uint8_t>((time.tm_sec / 10) << 4) | static_cast<uint8_t>(time.tm_sec % 10);
    buffer[af_minutes] = static_cast<uint8_t>((time.tm_min / 10) << 4) | static_cast<uint8_t>(time.tm_min % 10);
    buffer[af_hours] = static_cast<uint8_t>((time.tm_hour / 10) << 4) | static_cast<uint8_t>(time.tm_hour % 10);
    buffer[af_day] = static_cast<uint8_t>((time.tm_wday + 1) % 10);
    buffer[af_date] = static_cast<uint8_t>((time.tm_mday / 10) << 4) | static_cast<uint8_t>(time.tm_mday % 10);
    buffer[af_month_and_century] = static_cast<uint8_t>((time.tm_year / 100) << 7) | static_cast<uint8_t>(((time.tm_mon + 1) / 10) << 4) | static_cast<uint8_t>((time.tm_mon + 1) % 10);
    buffer[af_year] = static_cast<uint8_t>(((time.tm_year % 100) / 10) << 4) | static_cast<uint8_t>(((time.tm_year % 100) % 10));
}
