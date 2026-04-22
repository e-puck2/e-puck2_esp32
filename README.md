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

The e-puck 2.2 integrates additional 2 MB of PSRAM, some configurations are needed to use it.

#### e-puck 2.0 and e-puck 2.1 configuration
1. Specify `set(MICROPY_BOARD EPUCK_20)` in `components/mp_component/CMakeLists.txt`
2. `idf.py menuconfig`
   1. Select the partition table to be `components/mp_component/micropython/ports/esp32/boards/EPUCK_20/partitions-4MiB.csv`
   2. Select 4 MB for flash size
4. Specify `EPUCK20` in `components/mp_component/CMakeLists.txt`

#### e-puck 2.2 configuration
1. Specify `set(MICROPY_BOARD EPUCK_22)` in `components/mp_component/CMakeLists.txt`
2. `idf.py menuconfig`
   1. Select the partition table to be `components/mp_component/micropython/ports/esp32/boards/EPUCK_22/partitions-8MiB.csv`
   2. Select 8 MB for flash size
3. Specify `EPUCK22` in `components/mp_component/CMakeLists.txt`

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
3.  Run the **build command** using the ESP-IDF build system:
    ```bash
    idf.py build
    ```
