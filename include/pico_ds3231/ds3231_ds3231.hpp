#pragma once
#include "ds3231_registers.hpp"
#include "ds3231_traits.hpp"
#include "ds3231_datetime.hpp"
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/binary_info.h>
#include <pico/error.h>
#include <cstring>

#if defined(PICO_RP2040) && PICO_RP2040
#include <sys/time.h>
#include <cerrno>
#endif

namespace ds3231 {
    constexpr float temperature_min = -40;

    constexpr float temperature_max = 85;

    constexpr float invalid_temperature_sentinel = -41;

    constexpr bool valid_temperature(const float& temperature) {
        return temperature_min <= temperature && temperature <= temperature_max;
    }

    static inline bool is_i2c_initialized(i2c_inst_t *i2c) {
        return (i2c_get_hw(i2c)->enable & I2C_IC_ENABLE_ENABLE_BITS) != 0;
    }

    enum class Result : uint8_t {
        success = 0,
        unknown_error,
        no_device,
        incomplete_read,
        incomplete_write,
        timeout,
        buffer_too_small,
        buffer_too_large,
        lost_power,
        invalid_register_length,
        invalid_datetime,
    };

    template <typename TraitsDefinition = DefaultTraits>
    class BasicDS3231 {
    public:
        using Traits = TraitsDefinition;

        mutable Traits traits;

        BasicDS3231()
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

        Result now(DateTime& dt) const {
            Result result;

            RegisterBuffer<Register::timekeeping> buffer;
            if ((result = load<Register::timekeeping>(buffer)) != Result::success) {
                return result;
            }

            RegisterDeclaration<Register::timekeeping>::buffer_to_tm(buffer, dt.time);
            return Result::success;
        }

        DateTime now() const {
            DateTime dt;
            now(dt);
            return dt;
        }

        Result adjust(const DateTime& dt) {
            Result result;
            RegisterBuffer<Register::timekeeping> dt_buffer;
            RegisterDeclaration<Register::timekeeping>::tm_to_buffer(dt.time, dt_buffer);
            if ((result = put<Register::timekeeping>(dt_buffer)) != Result::success) {
                return result;
            }
            return Result::success;
        }

        float temperature() const {
            float out;
            if (temperature(out) != Result::success) {
                return invalid_temperature_sentinel;
            }
            return out;
        }

        Result temperature(float& out) const {
            RegisterBuffer<Register::temperature> buffer;
            Result result = load<Register::temperature>(buffer);
            if (result != Result::success) {
                return result;
            }
            out = RegisterDeclaration<Register::temperature>::buffer_to_temperature(buffer);
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
            } else if (retval == PICO_ERROR_TIMEOUT) {
                return Result::timeout;
            }else if (retval < 0) {
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

            int retval = traits.i2c_write(traits.i2c_instance, traits.address, transit_buffer, register_length + 1, true);
            if (retval == PICO_ERROR_GENERIC) {
                return Result::no_device;
            } else if (retval == PICO_ERROR_TIMEOUT) {
                return Result::timeout;
            } else if (retval < 0) {
                return Result::unknown_error;
            } else if (retval != register_length + 1) {
                return Result::incomplete_write;
            }

            return Result::success;
        }

    private:
        bool _initialized;
    };

    template <Register _register, typename Traits>
    Result load(const BasicDS3231<Traits>& ds3231, RegisterBuffer<_register>& buffer) {
        return ds3231.template load<_register>(buffer);
    }

    template <Register _register, typename Traits>
    Result put(BasicDS3231<Traits>& ds3231, RegisterBuffer<_register>& buffer) {
        return ds3231.template put<_register>(buffer);
    }

#if defined(PICO_RP2040) && PICO_RP2040
    template <typename Traits>
    Result sync_system_clocks(const BasicDS3231<Traits>& rtc_device) {
        RegisterBuffer<Register::timekeeping> buffer;
        Result result = rtc_device.template load<Register::timekeeping>(buffer);
        if (result != Result::success) {
            return result;
        }

        DateTime dt;
        RegisterDeclaration<Register::timekeeping>::buffer_to_tm(buffer, dt.time);

        ::datetime_t pico_raw = dt.to_pico_datetime();
        rtc_init();
        if (!rtc_set_datetime(&pico_raw)) {
            return Result::invalid_datetime;
        }
        sleep_us(64);

        struct timeval tv = { .tv_sec = dt.to_epoch(), .tv_usec = 0 };
        if (settimeofday(&tv, nullptr) != 0) {
            if (errno == EINVAL) {
                return Result::invalid_datetime;
            }
            return Result::unknown_error;
        }

        return Result::success;
    }
#endif

    template <typename Traits>
    static inline Result initialize_clock_time(BasicDS3231<Traits>& ds3231) {
        Result result;
        if ((result = ds3231.initialize()) != Result::success) {
            return result;
        }

        RegisterBuffer<Register::control_and_status> cas_buffer;
        if ((result = ds3231.template load<Register::control_and_status>(cas_buffer)) != Result::success) {
            return result;
        }


        RegisterBuffer<Register::timekeeping> dt_buffer;
        if ((result = ds3231.template load<Register::timekeeping>(dt_buffer)) != Result::success) {
            return result;
        }

        DateTime dt;
        RegisterDeclaration<Register::timekeeping>::buffer_to_tm(dt_buffer, dt.time);
        RegisterDeclaration<Register::control_and_status>::Flags flags = RegisterDeclaration<Register::control_and_status>::buffer_to_flags(cas_buffer);

        if (flags.oscillator_stop_flag) {
            if ((result = ds3231.adjust(compile_time)) != Result::success) {
                return result;
            }

            if (flags.oscillator_stop_flag) {
                flags.oscillator_stop_flag = false;
                RegisterDeclaration<Register::control_and_status>::flags_to_buffer(flags, cas_buffer);
                if ((result = ds3231.template put<Register::control_and_status>(cas_buffer)) != Result::success) {
                    return result;
                }
            }

            return Result::lost_power;
        }

        return Result::success;
    }

    using DS3231 = BasicDS3231<DefaultTraits>;
}
