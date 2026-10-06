#pragma once
#include <cstdint>

#if !defined(_PICO_DS3231_MAJOR) || !defined(_PICO_DS3231_MINOR) || !defined(_PICO_DS3231_PATCH)
#warning "Pico DS3231 library version macros are missing! Make sure you are building via the provided CMake architecture. Else, define them, using an invalid 0.0.0 version."
#define _PICO_DS3231_MAJOR (0)
#define _PICO_DS3231_MINOR (0)
#define _PICO_DS3231_PATCH (0)
#endif

namespace ds3231 {
    struct Version {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;

        constexpr Version(uint8_t major, uint8_t minor, uint8_t patch)
            : major(major), minor(minor), patch(patch) {}

        constexpr uint32_t packed() const {
            return (
                (static_cast<uint32_t>(major) << 16) |
                (static_cast<uint32_t>(minor) << 8) |
                (static_cast<uint32_t>(patch) << 0)
            );
        }

        constexpr bool operator>=(const Version& other) const {
            return packed() >= other.packed();
        }

        constexpr bool operator<=(const Version& other) const {
            return packed() <= other.packed();
        }

        constexpr bool operator<(const Version& other) const {
            return packed() < other.packed();
        }

        constexpr bool operator>(const Version& other) const {
            return packed() > other.packed();
        }

        constexpr bool operator==(const Version& other) const {
            return packed() == other.packed();
        }

        constexpr bool operator!=(const Version& other) const {
            return packed() != other.packed();
        }

        constexpr operator uint32_t() const {
            return packed();
        }

        constexpr bool valid() const {
            return packed() != 0;
        }
    };

    constexpr Version version {
        _PICO_DS3231_MAJOR,
        _PICO_DS3231_MINOR,
        _PICO_DS3231_PATCH,
    };
}
