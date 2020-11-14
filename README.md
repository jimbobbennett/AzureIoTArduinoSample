# Azure IoT Hub and Arduino Sample

This repo contains sample code for using Azure IoT Hub with Arduino boards.

## The sample

This code is written using [PlatformIO](https://platformio.org) in [Visual Studio Code](https://code.visualstudio.com/?WT.mc_id=academic-10991-jabenn). It targets an [ESP32-Azure IoT Kit](https://www.espressif.com/en/products/devkits/esp32-azure-kit/overview), an ESP32-based board with build in sensors and designed for Azure. You can use other boards, just update the [platformio.ini](./platformio.ini) file.

The sample demonstrates connecting to WiFi and setting the time, then connecting to IoT Hub and sending telemetry every few seconds. It will also respond to device twin updates and direct methods.

> At the time of writing this, handling cloud to device messages is not supported as it appears to be broken in the core SDK.

## Building and running the sample

At the time of writing, all the necessary Arduino libraries for Azure are not available from the Arduino library manager. Watch [this issue](https://github.com/arduino/Arduino/issues/10952) to see when the last library is available and they can all be installed from the library manager.

This means that you need to install the libraries manually.

1. Ensure you have Python, Visual Studio Code and the PlatformIO VS Code extension installed

1. Clone the [Azure Arduino IoT Pal](https://github.com/Azure/azure-iot-pal-arduino/) GitHub repo

1. Open a terminal or command prompt and navigate to the `build_all` folder in that repo

1. Run the following command to build the libraries:

    ```sh
    python3 make_sdk.py -o build
    ```

1. This will compile the libraries and put them in the `build` folder

1. Clone this repo

1. Copy the 5 `AzureIoT*` folders from the new `build` folder in the pal repo into the `lib` folder in the clone of this repo

1. Open this repo in VS Code, plug in an ESP32 board and upload the code using PlatformIO by launching the command palette and selecting **PlatformIO: Upload**. You may need to hold the *boot* button on your ESP32 device during the code upload to successfully copy the code over depending on your board.