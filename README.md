# Pool Controller System

The Pool Controller System is an automated solution designed to manage the operation of various pool components including pumps, heaters, UV systems, and lights. Built on a robust hardware platform utilizing Ethernet connectivity, NTP for time synchronization, and Flash storage for configuration retention, this system aims to provide an efficient and user-friendly way to maintain optimal pool conditions year-round.

## Features

- **Automated Control**: Manage your pool's pump, heater, UV system, and lights based on configurable schedules.
- **Time Synchronization**: Utilizes NTP to ensure accurate operation times.
- **Web Interface**: Configure settings and control your pool's components remotely via a built-in web server.
- **MDNS Support**: Easy access to the web interface through MDNS naming.
- **Flash Storage**: Settings are saved to flash memory, ensuring they persist across reboots.
- **Debugging Mode**: Optional serial output for debugging purposes.

## Hardware Requirements

- MCU or development board with Ethernet connectivity
- External relays for controlling pool hardware (pump, heater, UV, lights)
- Ethernet shield or module compatible with the MCU
- Optional: USB storage for logging (when compiled with `LogToUSBDrive` directive)

## Software Dependencies

- [NTPClient](https://github.com/arduino-libraries/NTPClient)
- [TimeLib](https://github.com/PaulStoffregen/Time)
- [Ethernet](https://www.arduino.cc/en/Reference/Ethernet) (or compatible library for your hardware)
- [FlashIAPBlockDevice](https://github.com/ARMmbed/mbed-os/tree/master/storage/blockdevice/FlashIAPBlockDevice) and [TDBStore](https://github.com/ARMmbed/mbed-os/tree/master/storage/kvstore/tdbstore) for storage
- [MDNS_Generic](https://github.com/khoih-prog/MDNS_Generic)

## Installation

1. **Hardware Setup**: Connect your hardware according to the schematics provided in the `schematics` directory (to be added by the user).
2. **Library Installation**: Install all required libraries through your IDE's library manager or manually include them into your project directory.
3. **Configuration**: Adjust the `#define` directives at the beginning of the main code file to match your hardware setup and preferences.
4. **Compilation and Upload**: Compile the code and upload it to your microcontroller using the appropriate IDE and tools for your hardware.

## Usage

After uploading the code and powering on your device, the system will start in its default state with the predefined schedules.

- **Web Interface**: Access the web interface through `http://poolcontroller.local` (replace `poolcontroller` with your actual MDNS hostname) to configure the schedules and settings.
- **Manual Override**: Use the physical user button to toggle the system's operational state.

## Configuration Guide

The Pool Controller System is designed to be highly configurable to meet various operational needs. Below is a detailed explanation of all user-configurable variables within the system. These can be adjusted either in the code before uploading or via the web interface, depending on the variable.

### Hardware Configuration Variables

- **LED and Relay Pins**: Define the microcontroller pins connected to LEDs and relays.
  - `const int LED[] = {LED_D0, LED_D1, LED_D2, LED_D3};`
  - `const int RELAY[] = {D0, D1, D2, D3};`
- **User Button**: Pin number for the user button.
  - `#define userButton BTN_USER`

### Network Configuration

- **Ethernet Configuration**: Setup parameters for Ethernet connectivity.
  - This is managed by the `Ethernet.begin();` method in the setup routine. Modify this call to include specific IP settings if needed.

### Time Configuration

- **NTP Settings**: Configure the NTP client for time synchronization.
  - The `NTPClient timeClient(NTPUdp);` instantiation can be modified to specify a different NTP server or time offset.

### Operational Variables

Variables affecting the system's operation can be configured through the web interface. The initial defaults are defined in the code and can be adjusted as needed:

- **Schedule Settings**: Time schedules for each component. Configurable via the web interface. Initial values:
  ```cpp
  TimeSetting defaultTimeSettings[] = {
    {"fromPump",    "08:00"},
    {"toPump",      "12:00"},
    // Add other settings similarly
  };


###Operational Flags:
bool debugFlag = true;: Enable or disable debug output to Serial.
bool isWinter = false;: Indicate if the system should operate in winter mode, affecting relay management logic.
Debugging and Logging
Debugging Mode: Controlled by the debugFlag variable. When enabled, various operational messages will be printed to the Serial interface.
USB Logging: If compiled with LogToUSBDrive directive, logs can be written to a USB storage device. This feature requires additional configuration:
#ifdef LogToUSBDrive: Uncomment and configure this section to enable USB logging.
MDNS Configuration
Hostname: The MDNS hostname can be set to make accessing the web interface more intuitive.
Modify the String hostname = "PoolController"; line to change the hostname.
Adjusting Operational Logic
The logic that controls the operation of the pool components based on time schedules and the state of the system (e.g., winter mode, manual override) is implemented within the manageRelays function and related areas of the code. Advanced users can modify this logic to introduce custom behaviors and controls.

###Saving Changes
After adjusting configuration variables through the web interface, ensure to save the changes. The system will write the new settings to flash memory, preserving them across restarts.

## Contributing

Contributions to the Pool Controller System are welcome! Please refer to the `CONTRIBUTING.md` file for guidelines on how to submit issues, feature requests, and pull requests.

## License

This project is licensed under the MIT License - see the `LICENSE` file for details.

## Acknowledgments

- Special thanks to all contributors and the open-source libraries used in this project.

---

For detailed information on setup, usage, and customization, please refer to the wiki pages or submit an issue if you encounter problems.
