# e-puck2_esp32
ESP32 firmware of e-puck2, isolated from esp-idf framework.

---

## Build Instructions

Follow these steps to set up the necessary tools and build the firmware.

### Initial Requirements (One-time Setup)

This section details how to get the ESP-IDF framework and the firmware source code.

#### 1. Get the Last Stable ESP-IDF

These steps clone and install the specific version of the Espressif IoT Development Framework (ESP-IDF) needed for the build.

1.  Clone the repository for version **v5.4** recursively:
    ```bash
    git clone -b v5.4 --recursive https://github.com/espressif/esp-idf.git esp-idf_5.4
    ```
2.  Change into the newly created directory (assuming the default path):
    ```bash
    cd ~/esp/esp-idf_5.4
    ```
3.  Run the installation script for the **esp32** target:
    ```bash
    ./install.sh esp32
    ```

#### 2. Get the Latest Firmware Source Code

These steps clone the specific branch of the firmware and initialize its submodules.

1.  Clone the `micropython` branch of the firmware repository:
    ```bash
    git clone -b micropython https://github.com/e-puck2/e-puck2_esp32.git e-puck2_esp32_micropython
    ```
2.  Change into the firmware directory:
    ```bash
    cd e-puck2_esp32_micropython
    ```
3.  Initialize the submodules:
    ```bash
    git submodule init
    ```
4.  Update the submodules:
    ```bash
    git submodule update
    ```
### Firmware configuration

The board is selected when building (see below), no manual configuration is needed:

| Robot | `BOARD` | Flash | Partition table |
|---|---|---|---|
| e-puck 2.0 and e-puck 2.1 | `EPUCK_20` | 4 MB | `boards/EPUCK_20/partitions-4MiB.csv` |
| e-puck 2.2 (additional 2 MB of PSRAM) | `EPUCK_22` | 8 MB | `boards/EPUCK_22/partitions-8MiB.csv` |

The partition tables are in `components/mp_component/micropython/ports/esp32/boards/`.

### Firmware Build

Once the requirements are set up, you can build the firmware with these steps.

1.  Source the **ESP-IDF environment variables** to make the build tools available:
    ```bash
    . $HOME/esp/esp-idf_5.4/export.sh
    ```
2.  Move to the **firmware directory** (e.g., `e-puck2_esp32_micropython`):
    ```bash
    # Example:
    # cd $HOME/e-puck2_esp32_micropython 
    ```
3.  Run the **build command** using the ESP-IDF build system, specifying the board with `BOARD`
    (`EPUCK_20` for e-puck 2.0 and 2.1, `EPUCK_22` for e-puck 2.2). Using a different build directory (`-B`)
    for each board avoids a full rebuild when switching board:
    ```bash
    # e-puck 2.0 and 2.1 (4 MB flash)
    idf.py -B build_epuck20 -DBOARD=EPUCK_20 build
    # e-puck 2.2 (8 MB flash + PSRAM)
    idf.py -B build_epuck22 -DBOARD=EPUCK_22 build
    ```
    If `BOARD` is missing or invalid the build stops with an error.
    The board is saved in the build directory, so the following commands only need `-B`
    (e.g. `idf.py -B build_epuck22 -p /dev/ttyUSB0 flash monitor`).

    The ESP-IDF configuration (`<build dir>/sdkconfig.<BOARD>`) is generated from `sdkconfig.defaults` (common options)
    and `sdkconfig.defaults.<BOARD>` (flash size and partition table). Changes made with `idf.py menuconfig` last until
    `idf.py fullclean`: to make them permanent add them to the `sdkconfig.defaults*` files, then run `idf.py fullclean`.

    When a robot is flashed with a firmware of the other board, the partition table changes and MicroPython reports
    "The filesystem appears to be corrupted": erase the flash before flashing (the files on the robot are lost):
    ```bash
    idf.py -B build_epuck20 -p /dev/ttyUSB0 erase-flash flash
    ```
