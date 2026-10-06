cmake_minimum_required(VERSION 3.13...3.27)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(pico_ds3231 VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB_RECURSE LIB_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
)

add_library(pico_ds3231 STATIC ${LIB_SOURCES})

target_include_directories(pico_ds3231
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src"
)

target_link_libraries(pico_ds3231 PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_rtc
)

target_compile_definitions(pico_ds3231 PUBLIC
    _PICO_DS3231_MAJOR=${pico_ds3231_VERSION_MAJOR}
    _PICO_DS3231_MINOR=${pico_ds3231_VERSION_MINOR}
    _PICO_DS3231_PATCH=${pico_ds3231_VERSION_PATCH}
)
