#pragma once
#include <ctime>
#include <hardware/rtc.h>

namespace ds3231 {
    constexpr const char compilation_months[12][4] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };

    class DateTime {
    public:
        tm time;

        constexpr DateTime(const char* date, const char* time_str)
            : time({})
        {
            int extracted_month = 0;
            for (int i = 0; i < 12; ++i) {
                if (date[0] == compilation_months[i][0] &&
                    date[1] == compilation_months[i][1] &&
                    date[2] == compilation_months[i][2]) {
                    extracted_month = i;
                    break;
                }
            }
            time.tm_mon = extracted_month;

            if (date[4] == ' ') {
                time.tm_mday = date[5] - '0';
            } else {
                time.tm_mday = (date[4] - '0') * 10 + (date[5] - '0');
            }

            int full_year = (date[7] - '0') * 1000 +
                            (date[8] - '0') * 100 +
                            (date[9] - '0') * 10 +
                            (date[10] - '0');
            time.tm_year = full_year - 1900;

            time.tm_hour = (time_str[0] - '0') * 10 + (time_str[1] - '0');
            time.tm_min  = (time_str[3] - '0') * 10 + (time_str[4] - '0');
            time.tm_sec  = (time_str[6] - '0') * 10 + (time_str[7] - '0');

            time.tm_isdst = -1;
        }

        DateTime();

        DateTime(const tm& time);

        DateTime(int year, int month, int day, int hour = 0, int min = 0, int sec = 0, int wday = -1);

        DateTime(std::time_t epoch);

        DateTime(const ::datetime_t& pico_dt);

        std::time_t to_epoch() const;

        ::datetime_t to_pico_datetime() const;
    };

    constexpr DateTime compile_time = DateTime(__DATE__, __TIME__);
}
