# This is the common ESP-IDF "main component" CMakeLists.txt contents for MicroPython.
#
# This file is included directly from a main_${IDF_TARGET}/CMakeLists.txt file
# (or included from an out-of-tree main component CMakeLists.txt for out-of-tree
# builds.)

message(STATUS "Configuring MP component")

# Set location of base MicroPython directory.
if(NOT MICROPY_DIR)
    #get_filename_component(MICROPY_DIR ${CMAKE_CURRENT_LIST_DIR}/micropython ABSOLUTE)
    set(MICROPY_DIR ${CMAKE_CURRENT_LIST_DIR}/micropython) # The mpy submodule's directory
endif()


# Set the e-puck board and variant if it's not already set.
if(NOT MICROPY_BOARD)
    set(MICROPY_BOARD EPUCK_20)
    #set(MICROPY_BOARD EPUCK_22)
endif()

# Set the board directory and check that it exists.
if(NOT MICROPY_BOARD_DIR)
    set(MICROPY_BOARD_DIR ${MICROPY_DIR}/ports/esp32/boards/${MICROPY_BOARD})
endif()

# Set location of the ESP32 port directory.
if(NOT MICROPY_PORT_DIR)
    #get_filename_component(MICROPY_PORT_DIR ${MICROPY_DIR}/ports/esp32 ABSOLUTE)
    set(MICROPY_PORT_DIR ${MICROPY_DIR}/ports/esp32)
endif()

# Set location of custom manifest file.
set(MICROPY_FROZEN_MANIFEST ${CMAKE_CURRENT_LIST_DIR}/manifest.py)

set(USER_C_MODULES
    ${MICROPY_DIR}/lib/ulab/code/micropython.cmake
)

# RISC-V specific inclusions
if(CONFIG_IDF_TARGET_ARCH_RISCV)
    list(APPEND MICROPY_SOURCE_LIB
        ${MICROPY_DIR}/shared/runtime/gchelper_native.c
        ${MICROPY_DIR}/shared/runtime/gchelper_rv32i.s
    )
endif()

if(NOT DEFINED MICROPY_PY_TINYUSB)
    if(CONFIG_IDF_TARGET_ESP32S2 OR CONFIG_IDF_TARGET_ESP32S3)
        set(MICROPY_PY_TINYUSB ON)
    endif()
endif()

# Enable error text compression by default.
if(NOT MICROPY_ROM_TEXT_COMPRESSION)
    set(MICROPY_ROM_TEXT_COMPRESSION ON)
endif()

# Include core source components.
include(${MICROPY_DIR}/py/py.cmake)

# CMAKE_BUILD_EARLY_EXPANSION is set during the component-discovery phase of
# `idf.py build`, so none of the extmod/usermod (and in reality, most of the
# micropython) rules need to happen. Specifically, you cannot invoke add_library.
if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    # Enable extmod components that will be configured by extmod.cmake.
    # A board may also have enabled additional components.
    if (NOT DEFINED MICROPY_PY_BTREE)
        set(MICROPY_PY_BTREE ON)
    endif()

    include(${MICROPY_DIR}/py/usermod.cmake)
    include(${MICROPY_DIR}/extmod/extmod.cmake)
endif()

list(APPEND MICROPY_QSTRDEFS_PORT
    ${MICROPY_PORT_DIR}/qstrdefsport.h
)

list(APPEND MICROPY_SOURCE_SHARED
    ${MICROPY_DIR}/shared/readline/readline.c
    ${MICROPY_DIR}/shared/netutils/netutils.c
    ${MICROPY_DIR}/shared/timeutils/timeutils.c
    ${MICROPY_DIR}/shared/runtime/interrupt_char.c
    ${MICROPY_DIR}/shared/runtime/mpirq.c
    ${MICROPY_DIR}/shared/runtime/stdout_helpers.c
    ${MICROPY_DIR}/shared/runtime/sys_stdio_mphal.c
    ${MICROPY_DIR}/shared/runtime/pyexec.c
)

list(APPEND MICROPY_SOURCE_LIB
    ${MICROPY_DIR}/lib/littlefs/lfs1.c
    ${MICROPY_DIR}/lib/littlefs/lfs1_util.c
    ${MICROPY_DIR}/lib/littlefs/lfs2.c
    ${MICROPY_DIR}/lib/littlefs/lfs2_util.c
    ${MICROPY_DIR}/lib/mbedtls_errors/esp32_mbedtls_errors.c
    ${MICROPY_DIR}/lib/oofatfs/ff.c
    ${MICROPY_DIR}/lib/oofatfs/ffunicode.c
)

list(APPEND MICROPY_SOURCE_DRIVERS
#    ${MICROPY_DIR}/drivers/bus/softspi.c
#    ${MICROPY_DIR}/drivers/dht/dht.c
)

if(MICROPY_PY_TINYUSB)
    string(TOUPPER OPT_MCU_${IDF_TARGET} tusb_mcu)

    list(APPEND MICROPY_DEF_TINYUSB
        CFG_TUSB_MCU=${tusb_mcu}
    )

    list(APPEND MICROPY_SOURCE_TINYUSB
        ${MICROPY_DIR}/shared/tinyusb/mp_usbd.c
        ${MICROPY_DIR}/shared/tinyusb/mp_usbd_cdc.c
        ${MICROPY_DIR}/shared/tinyusb/mp_usbd_descriptor.c
        ${MICROPY_DIR}/shared/tinyusb/mp_usbd_runtime.c
    )

    list(APPEND MICROPY_INC_TINYUSB
        ${MICROPY_DIR}/shared/tinyusb/
    )
endif()

list(APPEND MICROPY_SOURCE_PORT
    panichandler.c
    #adc.c
    #main.c
    ppp_set_auth.c
    uart.c
    #usb.c
    #usb_serial_jtag.c
    gccollect.c
    mphalport.c
    fatfs_port.c
    help.c
    #machine_bitstream.c
    machine_timer.c
    machine_pin.c
    machine_touchpad.c
    #machine_dac.c
    #machine_i2c.c
    network_common.c
    #network_lan.c
    network_ppp.c
    network_wlan.c
    #mpnimbleport.c
    modsocket.c
    lwip_patch.c
    modesp.c
    esp32_nvs.c
    esp32_partition.c
    #esp32_pcnt.c
    esp32_rmt.c
    esp32_ulp.c
    modesp32.c
    #machine_hw_spi.c
    mpthreadport.c
    machine_rtc.c
    #machine_sdcard.c
    modespnow.c
    modepuck2.c
)
list(TRANSFORM MICROPY_SOURCE_PORT PREPEND ${MICROPY_PORT_DIR}/)
list(APPEND MICROPY_SOURCE_PORT ${CMAKE_BINARY_DIR}/pins.c)

list(APPEND MICROPY_SOURCE_QSTR
    ${MICROPY_SOURCE_PY}
    ${MICROPY_SOURCE_EXTMOD}
    ${MICROPY_SOURCE_USERMOD}
    ${MICROPY_SOURCE_SHARED}
    ${MICROPY_SOURCE_LIB}
    ${MICROPY_SOURCE_PORT}
    ${MICROPY_SOURCE_BOARD}
    ${MICROPY_SOURCE_TINYUSB}
    ${CMAKE_CURRENT_LIST_DIR}/mp_component.c
)

list(APPEND IDF_COMPONENTS
    app_update
    bootloader_support
    bt
    driver
    #esp_adc
    esp_app_format
    esp_common
    #esp_eth
    esp_event
    esp_hw_support
    esp_netif
    esp_partition
    esp_pm
    esp_psram
    esp_ringbuf
    esp_rom
    esp_system
    esp_timer
    esp_wifi
    freertos
    hal
    heap
    log
    lwip
    mbedtls
    mdns
    newlib
    nvs_flash
    #sdmmc
    soc
    spi_flash
    ulp
    #usb
    vfs
)

# Provide the default LD fragment if not set
if (MICROPY_USER_LDFRAGMENTS)
    set(MICROPY_LDFRAGMENTS ${MICROPY_USER_LDFRAGMENTS})
endif()

if (UPDATE_SUBMODULES)
    # ESP-IDF checks if some paths exist before CMake does. Some paths don't
    # yet exist if this is an UPDATE_SUBMODULES pass on a brand new checkout, so remove
    # any path which might not exist yet. A "real" build will not set UPDATE_SUBMODULES.
    unset(MICROPY_SOURCE_TINYUSB)
    unset(MICROPY_SOURCE_EXTMOD)
    unset(MICROPY_SOURCE_LIB)
    unset(MICROPY_INC_TINYUSB)
    unset(MICROPY_INC_CORE)
endif()

# Register the main IDF component.
idf_component_register(
    SRCS
        ${MICROPY_SOURCE_PY}
        ${MICROPY_SOURCE_EXTMOD}
        ${MICROPY_SOURCE_SHARED}
        ${MICROPY_SOURCE_LIB}
        ${MICROPY_SOURCE_DRIVERS}
        ${MICROPY_SOURCE_PORT}
        ${MICROPY_SOURCE_BOARD}
        ${MICROPY_SOURCE_TINYUSB}
        ${CMAKE_CURRENT_LIST_DIR}/mp_component.c
    INCLUDE_DIRS
        ${MICROPY_INC_CORE}
        ${MICROPY_INC_USERMOD}
        ${MICROPY_INC_TINYUSB}
        ${MICROPY_PORT_DIR}
        ${MICROPY_BOARD_DIR}
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_LIST_DIR}
    LDFRAGMENTS
        ${MICROPY_LDFRAGMENTS}
    REQUIRES
        ${IDF_COMPONENTS}
)

## If a board variant is specified, check that it exists.
#if(MICROPY_BOARD_VARIANT)
#    message(STATUS "MICROPY_BOARD_VARIANT specified: ${MICROPY_BOARD_VARIANT}")
#    if(NOT EXISTS ${MICROPY_BOARD_DIR}/mpconfigvariant_${MICROPY_BOARD_VARIANT}.cmake)
#        message(FATAL_ERROR "Invalid MICROPY_BOARD_VARIANT specified: ${MICROPY_BOARD_VARIANT}")
#    endif()
#endif()
include(${MICROPY_BOARD_DIR}/mpconfigboard.cmake)
#if(MICROPY_BOARD_VARIANT)
#    include(${MICROPY_BOARD_DIR}/mpconfigvariant_${MICROPY_BOARD_VARIANT}.cmake)
#endif()

# the following deifinition is not considered...why??
## If the variant defines a custom partition table, use it
#set(MICROPY_PARTITIONS_CSV "${MICROPY_BOARD_DIR}/partitions-4MiB.csv")
#
#if(EXISTS "${MICROPY_PARTITIONS_CSV}")
#    message(STATUS "Using MicroPython board variant partitions: ${MICROPY_PARTITIONS_CSV}")
#    # Tell ESP-IDF to use this partition table
#    set(PARTITION_TABLE_CUSTOM_FILENAME "${MICROPY_PARTITIONS_CSV}" CACHE INTERNAL "Board variant partition file")
#    set(PARTITION_TABLE_FILENAME "${MICROPY_PARTITIONS_CSV}" CACHE INTERNAL "Board variant partition file")
#endif()

# Set the MicroPython target as the current (main) IDF component target.
set(MICROPY_TARGET ${COMPONENT_TARGET})

# Define mpy-cross flags, for use with frozen code.
if(CONFIG_IDF_TARGET_ARCH_XTENSA)
    set(MICROPY_CROSS_FLAGS -march=xtensawin)
elseif(CONFIG_IDF_TARGET_ARCH_RISCV)
    set(MICROPY_CROSS_FLAGS -march=rv32imc)
endif()

# Set compile options for this port.
target_compile_definitions(${MICROPY_TARGET} PUBLIC
    ${MICROPY_DEF_COMPONENT}
    ${MICROPY_DEF_CORE}
    ${MICROPY_DEF_BOARD}
    ${MICROPY_DEF_TINYUSB}
    MICROPY_VFS_FAT=1
    MICROPY_VFS_LFS2=1
    FFCONF_H=\"${MICROPY_OOFATFS_DIR}/ffconf.h\"
    LFS1_NO_MALLOC LFS1_NO_DEBUG LFS1_NO_WARN LFS1_NO_ERROR LFS1_NO_ASSERT
    LFS2_NO_MALLOC LFS2_NO_DEBUG LFS2_NO_WARN LFS2_NO_ERROR LFS2_NO_ASSERT
    MICROPY_BOARD="EPUCK"
    MICROPY_VARIANT="EPUCK2"
)

# Disable some warnings to keep the build output clean.
target_compile_options(${MICROPY_TARGET} PUBLIC
    ${MICROPY_COMPILE_COMPONENT}
    -Wno-clobbered
    -Wno-deprecated-declarations
    -Wno-missing-field-initializers
)

# Additional include directories needed for private NimBLE headers.
target_include_directories(${MICROPY_TARGET} PUBLIC
    ${IDF_PATH}/components/bt/host/nimble/nimble
)

# Add additional extmod and usermod components.
if (MICROPY_PY_BTREE)
    target_link_libraries(${MICROPY_TARGET} $<TARGET_OBJECTS:micropy_extmod_btree>)
endif()
target_link_libraries(${MICROPY_TARGET} usermod)

# Extra linker options
# (when wrap symbols are in standalone files, --undefined ensures
# the linker doesn't skip that file.)
target_link_options(${MICROPY_TARGET} PUBLIC
  # Patch LWIP memory pool allocators (see lwip_patch.c)
  -Wl,--undefined=memp_malloc
  -Wl,--wrap=memp_malloc
  -Wl,--wrap=memp_free

  # Enable the panic handler wrapper
  -Wl,--undefined=esp_panic_handler
  -Wl,--wrap=esp_panic_handler
)

# Collect all of the include directories and compile definitions for the IDF components,
# including those added by the IDF Component Manager via idf_components.yaml.
foreach(comp ${__COMPONENT_NAMES_RESOLVED})
    micropy_gather_target_properties(__idf_${comp})
    micropy_gather_target_properties(${comp})
endforeach()

# Include the main MicroPython cmake rules.
include(${MICROPY_DIR}/py/mkrules.cmake)

# Generate source files for named pins (requires mkrules.cmake for MICROPY_GENHDR_DIR).

set(GEN_PINS_PREFIX "${MICROPY_PORT_DIR}/boards/pins_prefix.c")
set(GEN_PINS_MKPINS "${MICROPY_PORT_DIR}/boards/make-pins.py")
set(GEN_PINS_SRC "${CMAKE_BINARY_DIR}/pins.c")
set(GEN_PINS_HDR "${MICROPY_GENHDR_DIR}/pins.h")

if(EXISTS "${MICROPY_BOARD_DIR}/pins.csv")
    set(GEN_PINS_BOARD_CSV "${MICROPY_BOARD_DIR}/pins.csv")
    set(GEN_PINS_BOARD_CSV_ARG --board-csv "${GEN_PINS_BOARD_CSV}")
endif()

target_sources(${MICROPY_TARGET} PRIVATE ${GEN_PINS_HDR})

add_custom_command(
    OUTPUT ${GEN_PINS_SRC} ${GEN_PINS_HDR}
    COMMAND ${Python3_EXECUTABLE} ${GEN_PINS_MKPINS} ${GEN_PINS_BOARD_CSV_ARG}
        --prefix ${GEN_PINS_PREFIX} --output-source ${GEN_PINS_SRC} --output-header ${GEN_PINS_HDR}
    DEPENDS
        ${MICROPY_MPVERSION}
        ${GEN_PINS_MKPINS}
        ${GEN_PINS_BOARD_CSV}
        ${GEN_PINS_PREFIX}
    VERBATIM
    COMMAND_EXPAND_LISTS
)

message(STATUS "Configuring MP component DONE")