#pragma once
#include <cstdint>
#include <ctime>

namespace ds3231 {
    enum class Register : uint8_t {
        invalid,
        timekeeping,
        alarm_1,
        alarm_2,
        control_and_status,
        aging_offset,
        temperature
    };

    template <Register>
    struct RegisterDeclaration;

    template <>
    struct RegisterDeclaration<Register::invalid> {
        static constexpr uint8_t register_start = 0x00;

        static constexpr uint8_t register_length = 0x00;

        enum AddressFunctionOffset : uint8_t {};
    };

    template <>
    struct RegisterDeclaration<Register::timekeeping> {
        static constexpr uint8_t register_start = 0x00;

        static constexpr uint8_t register_length = 0x07;

        enum AddressFunctionOffset : uint8_t {
            af_seconds = 0x00,
            af_minutes = 0x01,
            af_hours = 0x02,
            af_day = 0x03,
            af_date = 0x04,
            af_month_and_century = 0x05,
            af_year = 0x06
        };

        static bool is_12_hour(const uint8_t (&buffer)[register_length]);

        static void buffer_to_tm(const uint8_t (&buffer)[register_length], tm& time);

        static tm buffer_to_tm(const uint8_t (&buffer)[register_length]) ;

        static void tm_to_buffer(const tm& time, uint8_t (&buffer)[register_length]);
    };

    template <uint8_t alarm_start>
    struct GenericAlarmRegisterDeclaration {
        static constexpr uint8_t register_start = alarm_start;

        static constexpr uint8_t register_length = 0x04;

        enum AddressFunctionOffset : uint8_t {
            af_seconds = 0x00,
            af_minutes = 0x01,
            af_hours = 0x02,
            af_day_or_date = 0x03
        };
        static constexpr uint8_t mask_bit = 0x80;

        static constexpr uint8_t dy_dt_bit = 0x40;

        static inline bool is_12_hour(const uint8_t(&buffer)[register_length]) {
            return (buffer[af_hours] & 0b01000000) > 0;
        }

        static inline void buffer_to_tm(const uint8_t (&buffer)[register_length], tm& time, bool& is_alarm_per_weekday, uint8_t& alarm_mask) {
            alarm_mask =  (buffer[af_seconds] & mask_bit) ? 0x01 : 0x00;
            alarm_mask |= (buffer[af_minutes] & mask_bit) ? 0x02 : 0x00;
            alarm_mask |= (buffer[af_hours] & mask_bit) ? 0x04 : 0x00;
            alarm_mask |= (buffer[af_day_or_date] & mask_bit) ? 0x08 : 0x00;

            time.tm_sec = buffer[af_seconds] & 0b00001111;
            time.tm_sec += 10 * ((buffer[af_seconds] & 0b01110000) >> 4);

            time.tm_min = buffer[af_minutes] & 0b00001111;
            time.tm_min += 10 * ((buffer[af_minutes] & 0b01110000) >> 4);

            time.tm_hour = buffer[af_hours] & 0b00001111;
            if (is_12_hour(buffer)) {
                time.tm_hour += 10 * ((buffer[af_hours] & 0b00010000) >> 4);
                const bool is_pm = (buffer[af_hours] & 0b00100000) > 0;
                if (is_pm && time.tm_hour != 12) {
                    time.tm_hour += 12;
                } else if (!is_pm && time.tm_hour == 12) {
                    time.tm_hour = 0;
                }
            } else {
                time.tm_hour += 10 * ((buffer[af_hours] & 0b00110000) >> 4);
            }

            is_alarm_per_weekday = (buffer[af_day_or_date] & dy_dt_bit) > 0;

            if (is_alarm_per_weekday) {
                time.tm_wday = (buffer[af_day_or_date] & 0b00000111) - 1;
                time.tm_mday = 0;
            } else {
                time.tm_mday = buffer[af_day_or_date] & 0b00001111;
                time.tm_mday += 10 * ((buffer[af_day_or_date] & 0b00110000) >> 4);
                time.tm_wday = -1;
            }
        }

        static inline tm buffer_to_tm(const uint8_t (&buffer)[register_length], bool& is_alarm_per_weekday, uint8_t& alarm_mask) {
            tm time;
            buffer_to_tm(buffer, time, is_alarm_per_weekday, alarm_mask);
            return time;
        }

        static inline void tm_to_buffer(const tm& time, bool is_alarm_per_weekday, uint8_t alarm_mask, uint8_t (&buffer)[register_length]) {
            buffer[af_seconds] = static_cast<uint8_t>((time.tm_sec / 10) << 4) | static_cast<uint8_t>(time.tm_sec % 10);
            buffer[af_minutes] = static_cast<uint8_t>((time.tm_min / 10) << 4) | static_cast<uint8_t>(time.tm_min % 10);
            buffer[af_hours] = static_cast<uint8_t>((time.tm_hour / 10) << 4) | static_cast<uint8_t>(time.tm_hour % 10);

            if (is_alarm_per_weekday) {
                buffer[af_day_or_date] = dy_dt_bit | static_cast<uint8_t>((time.tm_wday + 1) & 0x07);
            } else {
                buffer[af_day_or_date] = static_cast<uint8_t>((time.tm_mday / 10) << 4) | static_cast<uint8_t>(time.tm_mday % 10);
            }

            if (alarm_mask & 0x01) {
                buffer[af_seconds] |= mask_bit;
            }

            if (alarm_mask & 0x02) {
                buffer[af_minutes] |= mask_bit;
            }

            if (alarm_mask & 0x04) {
                buffer[af_hours] |= mask_bit;
            }

            if (alarm_mask & 0x08) {
                buffer[af_day_or_date] |= mask_bit;
            }
        }
    };

    template <>
    struct RegisterDeclaration<Register::alarm_1> : public GenericAlarmRegisterDeclaration<0x07> {};

    template <>
    struct RegisterDeclaration<Register::alarm_2> : public GenericAlarmRegisterDeclaration<0x0B> {};

    template <>
    struct RegisterDeclaration<Register::control_and_status> {
        static constexpr uint8_t register_start = 0x0E;

        static constexpr uint8_t register_length = 0x02;

        enum AddressFunctionOffset : uint8_t {
            af_control = 0x00,
            af_control_and_status = 0x01
        };

        enum ControlBit : uint8_t {
            b_alarm_1_interrupt_enable = 0x01,
            b_alarm_2_interrupt_enable = 0x02,
            b_interrupt_control = 0x02,
            b_rate_select_1 = 0x03,
            b_rate_select_2 = 0x04,
            b_convert_temperature = 0x05,
            b_battery_backed_square_wave_enable = 0x06,
            b_enable_oscillator = 0x07,
        };

        enum ControlAndStatusBit : uint8_t {
            b_alarm_1_flag = 0x00,
            b_alarm_2_flag = 0x01,
            b_busy = 0x02,
            b_enable_32khz_output = 0x03,
            b_oscillator_stop_flag = 0x07
        };

        struct Flags {
            bool alarm_1_interrupt_enable;
            bool alarm_2_interrupt_enable;
            bool interrupt_control;
            bool rate_select_2;
            bool rate_select_1;
            bool convert_temperature;
            bool battery_backed_square_wave_enable;
            bool enable_oscillator;

            bool alarm_1_flag;
            bool alarm_2_flag;
            const bool busy;
            bool enable_32khz_output;
            bool oscillator_stop_flag;

            inline Flags(
                bool alarm_1_interrupt_enable,
                bool alarm_2_interrupt_enable,
                bool interrupt_control,
                bool rate_select_2,
                bool rate_select_1,
                bool convert_temperature,
                bool battery_backed_square_wave_enable,
                bool enable_oscillator,

                bool alarm_1_flag,
                bool alarm_2_flag,
                bool busy,
                bool enable_32khz_output,
                bool oscillator_stop_flag
            )
            :
                alarm_1_interrupt_enable(alarm_1_interrupt_enable),
                alarm_2_interrupt_enable(alarm_2_interrupt_enable),
                interrupt_control(interrupt_control),
                rate_select_2(rate_select_2),
                rate_select_1(rate_select_1),
                convert_temperature(convert_temperature),
                battery_backed_square_wave_enable(battery_backed_square_wave_enable),
                enable_oscillator(enable_oscillator),

                alarm_1_flag(alarm_1_flag),
                alarm_2_flag(alarm_2_flag),
                busy(busy),
                enable_32khz_output(enable_32khz_output),
                oscillator_stop_flag(oscillator_stop_flag)
            {}
        };

        static inline Flags buffer_to_flags(const uint8_t (&buffer)[register_length]) {
            return Flags(
                buffer[af_control] & b_alarm_1_interrupt_enable,
                buffer[af_control] & b_alarm_2_interrupt_enable,
                buffer[af_control] & b_interrupt_control,
                buffer[af_control] & b_rate_select_2,
                buffer[af_control] & b_rate_select_1,
                buffer[af_control] & b_convert_temperature,
                buffer[af_control] & b_battery_backed_square_wave_enable,
                buffer[af_control] & b_enable_oscillator,

                buffer[af_control_and_status] & b_alarm_1_flag,
                buffer[af_control_and_status] & b_alarm_2_flag,
                buffer[af_control_and_status] & b_busy,
                buffer[af_control_and_status] & b_enable_32khz_output,
                buffer[af_control_and_status] & b_oscillator_stop_flag
            );
        }

        static inline void set_bit(uint8_t& byte, const bool& value, const uint8_t& offset) {
            if (value) {
                byte |= (1 << offset);
            } else {
                byte &= ~(1 << offset);
            }
        }

        static void flags_to_buffer(const Flags& flags, uint8_t (&buffer)[register_length]) {
                set_bit(buffer[af_control], flags.alarm_1_interrupt_enable, b_alarm_1_interrupt_enable);
                set_bit(buffer[af_control], flags.alarm_2_interrupt_enable, b_alarm_2_interrupt_enable);
                set_bit(buffer[af_control], flags.interrupt_control, b_interrupt_control);
                set_bit(buffer[af_control], flags.rate_select_2, b_rate_select_2);
                set_bit(buffer[af_control], flags.rate_select_1, b_rate_select_1);
                set_bit(buffer[af_control], flags.convert_temperature, b_convert_temperature);
                set_bit(buffer[af_control], flags.battery_backed_square_wave_enable, b_battery_backed_square_wave_enable);
                set_bit(buffer[af_control], flags.enable_oscillator, b_enable_oscillator);

                set_bit(buffer[af_control_and_status], flags.alarm_1_flag, b_alarm_1_flag);
                set_bit(buffer[af_control_and_status], flags.alarm_2_flag, b_alarm_2_flag);
                set_bit(buffer[af_control_and_status], flags.enable_32khz_output, b_enable_32khz_output);
                set_bit(buffer[af_control_and_status], flags.oscillator_stop_flag, b_oscillator_stop_flag);
        }
    };

    template <>
    struct RegisterDeclaration<Register::aging_offset> {
        static constexpr uint8_t register_start = 0x10;

        static constexpr uint8_t register_length = 0x01;

        enum AddressFunctionOffset : uint8_t {
            af_aging_offset = 0x00
        };

        static inline int8_t buffer_to_aging_offset(const uint8_t (&buffer)[register_length]) {
            return static_cast<int8_t>(buffer[af_aging_offset]);
        }

        static inline void aging_offset_to_buffer(const int8_t& aging_offset, uint8_t (&buffer)[register_length]) {
            buffer[af_aging_offset] = static_cast<uint8_t>(aging_offset);
        }
    };

    template <>
    struct RegisterDeclaration<Register::temperature> {
        static constexpr uint8_t register_start = 0x11;

        static constexpr uint8_t register_length = 0x02;

        enum AddressFunctionOffset : uint8_t {
            af_temperature_msb = 0x00,
            af_temperature_lsb = 0x00
        };

        static inline float buffer_to_temperature(const uint8_t (&buffer)[register_length]) {
            return ((buffer[af_temperature_msb] << 8) | (buffer[af_temperature_msb] & 0b11000000)) / 256.0;
        }
    };

    constexpr uint8_t max_register_length = 0x12;

    template <Register _register>
    using RegisterBuffer = uint8_t[RegisterDeclaration<_register>::register_length];

    template <Register _register>
    using AddressFunctionOffset = typename RegisterDeclaration<_register>::AddressFunctionOffset;

    template <Register _register>
    constexpr uint8_t start() {
        static_assert(_register != Register::invalid, "Cannot use invalid register.");
        return RegisterDeclaration<_register>::register_start;
    }

    template <Register _register>
    constexpr uint8_t length() {
        static_assert(_register != Register::invalid, "Cannot use invalid register.");
        return RegisterDeclaration<_register>::register_length;
    }

    template <Register _register>
    constexpr uint8_t absolute_offset(AddressFunctionOffset<_register> function) {
        static_assert(_register != Register::invalid, "Cannot use invalid register.");
        static_assert(static_cast<uint8_t>(function) < length<_register>(), "Invalid register declaration, function offset cannot be length or greater.");
        return start<_register>() + static_cast<uint8_t>(function);
    }
}

