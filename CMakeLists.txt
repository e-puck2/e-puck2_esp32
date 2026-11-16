
# (Automatically converted from project Makefile by convert_to_cmake.py.)

# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

if("${IDF_TARGET}" STREQUAL "linux")
    set(COMPONENTS main)
endif()

# e-puck2 board selection, e.g.: idf.py -DBOARD=EPUCK_20 build
# The value is saved in the CMake cache of the build directory, so it must be given only the first time.
# Use a different build directory for each board to avoid full rebuilds when switching:
#   idf.py -B build_epuck20 -DBOARD=EPUCK_20 build
#   idf.py -B build_epuck22 -DBOARD=EPUCK_22 build
set(EPUCK2_BOARDS EPUCK_20 EPUCK_22)
set(BOARD "" CACHE STRING "e-puck2 board to build for (EPUCK_20 or EPUCK_22)")
set_property(CACHE BOARD PROPERTY STRINGS ${EPUCK2_BOARDS})
if(NOT BOARD IN_LIST EPUCK2_BOARDS)
    message(FATAL_ERROR "Board not specified or invalid (BOARD='${BOARD}'). Build with:\n"
                        "  idf.py -DBOARD=EPUCK_20 build   (e-puck2.0, 4 MB flash)\n"
                        "  idf.py -DBOARD=EPUCK_22 build   (e-puck2.2, 8 MB flash + PSRAM)")
endif()
message(STATUS "Building for e-puck2 board: ${BOARD}")

# MicroPython board (components/mp_component/micropython/ports/esp32/boards/<BOARD>).
set(MICROPY_BOARD ${BOARD})

# The ESP-IDF configuration is generated in the build directory from the common defaults plus the board
# specific ones (flash size and partition table). Options changed with "idf.py menuconfig" are kept until
# "idf.py fullclean": to make them permanent add them to sdkconfig.defaults (or sdkconfig.defaults.<BOARD>).
if(NOT SDKCONFIG)
    set(SDKCONFIG "${CMAKE_BINARY_DIR}/sdkconfig.${BOARD}")
endif()
set(SDKCONFIG_DEFAULTS "sdkconfig.defaults;sdkconfig.defaults.${BOARD}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP32_E-Puck_2)
