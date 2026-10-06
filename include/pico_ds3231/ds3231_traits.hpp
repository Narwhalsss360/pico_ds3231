#pragma once
#include <cstdint>
#include <hardware/i2c.h>

namespace ds3231 {
    constexpr uint8_t default_address = 0x68;

    template <uint8_t _address = default_address>
    struct MakeDefaultTraits {
        static constexpr uint32_t i2c_sda_pin = PICO_DEFAULT_I2C_SDA_PIN;

        static constexpr uint32_t i2c_scl_pin = PICO_DEFAULT_I2C_SCL_PIN;

        static constexpr i2c_inst_t* const i2c_instance = PICO_DEFAULT_I2C_INSTANCE();

        static constexpr uint8_t address = _address;

        int i2c_write(i2c_inst_t* i2c, uint8_t address, const uint8_t* source, size_t length, bool nostop) {
            return ::i2c_write_blocking(i2c, address, source, length, nostop);
        }

        int i2c_read(i2c_inst_t* i2c, uint8_t address, uint8_t* destination, size_t length, bool nostop) {
            return ::i2c_read_blocking(i2c, address, destination, length, nostop);
        }
    };

    using DefaultTraits = MakeDefaultTraits<default_address>;
}
