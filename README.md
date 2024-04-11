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

## Configuration

Edit the time settings within the web interface to customize the operational schedules for each pool component. You can define two separate schedules per component to accommodate different needs throughout the day.

## Contributing

Contributions to the Pool Controller System are welcome! Please refer to the `CONTRIBUTING.md` file for guidelines on how to submit issues, feature requests, and pull requests.

## License

This project is licensed under the MIT License - see the `LICENSE` file for details.

## Acknowledgments

- Special thanks to all contributors and the open-source libraries used in this project.

---

For detailed information on setup, usage, and customization, please refer to the wiki pages or submit an issue if you encounter problems.
