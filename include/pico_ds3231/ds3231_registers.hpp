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
            ad_day_or_date = 0x03
        };
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
            control = 0x00,
            control_and_status = 0x01
        };
    };

    template <>
    struct RegisterDeclaration<Register::aging_offset> {
        static constexpr uint8_t register_start = 0x10;

        static constexpr uint8_t register_length = 0x01;

        enum AddressFunctionOffset : uint8_t {
            aging_offset = 0x00
        };
    };

    template <>
    struct RegisterDeclaration<Register::temperature> {
        static constexpr uint8_t register_start = 0x11;

        static constexpr uint8_t register_length = 0x02;

        enum AddressFunctionOffset : uint8_t {
            temperature_msb = 0x00,
            temperature_lsb = 0x00
        };
    };

    constexpr uint8_t max_register_length = 0x12;

    template <Register _register>
    using RegisterBuffer = uint8_t[RegisterDeclaration<_register>::register_length];

    template <Register _register>
    using AddressFunctionOffset = typename RegisterDeclaration<_register>::AddressFunctionOffset;

    template <Register _register>
    constexpr uint8_t start() {
        static_assert(_register != Register::invalid, "Cannot use invalid register.");
        return RegisterDeclaration<_register>::start;
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

