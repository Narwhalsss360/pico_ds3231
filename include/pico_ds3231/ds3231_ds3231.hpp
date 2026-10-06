#pragma once
#include "ds3231_registers.hpp"
#include "ds3231_traits.hpp"
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/binary_info.h>
#include <pico/error.h>
#include <cstring>

namespace ds3231 {
    static inline bool is_i2c_initialized(i2c_inst_t *i2c) {
        return (i2c_get_hw(i2c)->enable & I2C_IC_ENABLE_ENABLE_BITS) != 0;
    }

    enum class Result : uint8_t {
        success = 0,
        unknown_error,
        no_device,
        incomplete_read,
        timeout,
        buffer_too_small,
        buffer_too_large,
        invalid_register_length
    };

    class DS3231 {
    public:
        using Traits = DefaultTraits;

        mutable Traits traits;

        DS3231()
            : traits(Traits()), _initialized(false) {}

        Result initialize() {
            if (_initialized) {
                return Result::success;
            }

            if (!is_i2c_initialized(traits.i2c_instance)) {
                i2c_init(traits.i2c_instance, 100 * 1000);
                gpio_set_function(traits.i2c_sda_pin, GPIO_FUNC_I2C);
                gpio_set_function(traits.i2c_scl_pin, GPIO_FUNC_I2C);
                gpio_pull_up(traits.i2c_sda_pin);
                gpio_pull_up(traits.i2c_scl_pin);
                bi_decl(bi_2pins_with_func(traits.i2c_sda_pin, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
            }

            return Result::success;
        }

        template <Register _register>
        Result load(RegisterBuffer<_register>& buffer) const {
            return load(start<_register>(), length<_register>(), buffer, length<_register>());
        }

        template <Register _register>
        Result put(const RegisterBuffer<_register>& buffer) {
            return put(start<_register>(), length<_register>(), buffer, length<_register>());
        }

    protected:
        Result load(const uint8_t& register_start, const uint8_t& register_length, uint8_t buffer[], uint8_t buffer_length) const {
            if (buffer_length < register_length) {
                return Result::buffer_too_small;
            }

            std::memset(buffer, 0, register_length);
            int retval = traits.i2c_write(traits.i2c_instance, traits.address, &register_start, 1, true);
            if (retval == PICO_ERROR_GENERIC) {
                return Result::no_device;
            } else if (retval < 0) {
                return Result::unknown_error;
            }

            retval = traits.i2c_read(traits.i2c_instance, traits.address, buffer, register_length, true);
            if (retval == PICO_ERROR_GENERIC) {
                return Result::no_device;
            } else if (retval < 0) {
                return Result::unknown_error;
            } else if (retval == PICO_ERROR_TIMEOUT) {
                return Result::timeout;
            } else if (retval != register_length) {
                return Result::incomplete_read;
            }

            return Result::success;
        }

        Result put(const uint8_t& register_start,const uint8_t& register_length, const uint8_t buffer[], uint8_t buffer_length) {
            if (register_length < buffer_length) {
                return Result::buffer_too_large;
            }

            if (register_length > max_register_length || register_length == 0) {
                return Result::invalid_register_length;
            }

            uint8_t transit_buffer[max_register_length + 1];
            std::memset(transit_buffer, 0, sizeof(transit_buffer));
            transit_buffer[0] = register_start;
            std::memcpy(transit_buffer + 1, buffer, register_length);

            int retval = traits.i2c_write(traits.i2c_instance, traits.address, transit_buffer, register_length, true);
            if (retval == PICO_ERROR_GENERIC) {
                return Result::no_device;
            } else if (retval < 0) {
                return Result::unknown_error;
            }

            return Result::success;
        }

    private:
        bool _initialized;
    };
}
