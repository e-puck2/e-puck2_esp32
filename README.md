# 🤖 e-puck2_esp32
ESP32 firmware of e-puck2, isolated from esp-idf framework.

---

## 🛠️ Build Instructions

Follow these steps to set up the necessary tools and build the firmware.

### Initial Requirements (One-time Setup)

This section details how to get the ESP-IDF framework and the firmware source code.

#### 1. Get the Last Stable ESP-IDF

These steps clone and install the specific version of the Espressif IoT Development Framework (ESP-IDF) needed for the build.

1.  Clone the repository for version **v5.4** recursively:
    ```bash
    git clone -b v5.4 --recursive [https://github.com/espressif/esp-idf.git](https://github.com/espressif/esp-idf.git) esp-idf_5.4
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
    git clone -b micropython [https://github.com/e-puck2/e-puck2_esp32.git](https://github.com/e-puck2/e-puck2_esp32.git) e-puck2_esp32_micropython
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

### ⚙️ Firmware Build

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
