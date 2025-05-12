# ASUS TUF M3 Mouse Driver

## Overview

The `asus_tuf_m3` driver is a custom HID-based kernel driver for the ASUS TUF Gaming M3 mouse. It replaces the generic Linux HID driver (`hid-generic`) to provide fine-grained control over mouse functionality via a character device interface. Key features include enabling/disabling individual buttons, scroll wheel, and movement axes, as well as adjusting scroll and cursor movement sensitivity.

### Supported Hardware

This driver is designed specifically for the ASUS TUF Gaming M3 mouse (USB VID: 0x0B05, PID: 0x18F3). Compatibility with other ASUS mice or firmware versions is not guaranteed.

### Advantages

- Lightweight alternative to ASUS's proprietary drivers.
- Programmatic control over button and axis functionality.
- Adjustable sensitivity settings directly from the kernel.

## Installation and Setup

### Kernel Requirements

- Linux kernel version 6.11.0-25-generic or later.
- Required kernel configurations: `CONFIG_HID`, `CONFIG_USB_HID`, `CONFIG_INPUT`.

### Dependencies

- `gcc`
- `make`
- `linux-headers` for your kernel version (e.g., `linux-headers-6.11.0-25-generic`)

Install dependencies on Ubuntu/Debian:

```bash
sudo apt update
sudo apt install gcc make linux-headers-$(uname -r)
```

### Build Instructions

1. Clone the repository (or extract the source tarball):

   ```bash
   git clone <repository-url>
   cd asus_tuf_m3
   ```

2. Build the driver:

   ```bash
   make
   ```

3. Load the driver into the kernel:

   ```bash
   sudo insmod asus_tuf_m3.ko
   ```

The driver automatically creates the device file `/dev/asus_tuf_m3` via `udev`. If the device file is missing, create it manually:

```bash
sudo mknod /dev/asus_tuf_m3 c <major> 0
```

Replace `<major>` with the major number assigned to the driver (check `cat /proc/devices | grep asus_tuf_m3`).

### Unloading the Driver

To unload the driver:

```bash
sudo rmmod asus_tuf_m3
```

### Troubleshooting

- **Module not found**: Ensure the kernel module (`asus_tuf_m3.ko`) is in the current directory.
- **Device file missing**: Verify the driver is loaded (`lsmod | grep asus_tuf_m3`) and create the device node manually if needed.
- **Permission denied**: Run user-space programs with `sudo` or adjust `/dev/asus_tuf_m3` permissions (`sudo chmod 666 /dev/asus_tuf_m3`).

## Driver Architecture

### HID Integration

The driver registers as a custom HID driver by defining a `struct hid_driver`. It matches the ASUS TUF M3 mouse based on its USB Vendor ID (VID) and Product ID (PID). The driver uses the Linux HID subsystem for core input event handling and extends it with a character device for `ioctl`-based control.

### Module Structure

- **Probe Function**: When the mouse is connected, the driver binds to the USB hub, initializes a `mouse` structure (containing HID device state), sets up supported input events (e.g., `EV_KEY` for buttons, `EV_REL` for movement/scroll), parses HID reports, and registers the character device `/dev/asus_tuf_m3`.
- **Input Handling**: The driver processes HID reports and translates them into input events, with optional filtering based on `ioctl` settings (e.g., disabled buttons).
- **IOCTL Interface**: Provides user-space control over button states, scroll wheel, movement axes, and sensitivity.

### Device Communication

The driver communicates with the mouse via USB HID reports. Sensitivity adjustments and button/axis toggles modify how HID reports are processed or forwarded to the input subsystem.

## IOCTL Interface

The driver exposes a character device (`/dev/asus_tuf_m3`) that supports the following `ioctl` commands. All commands return 0 on success or -1 on error, setting `errno` appropriately (e.g., `EINVAL` for invalid arguments, `ENOTTY` for unrecognized commands).

| Command | Purpose | Input/Output | Valid Range | Default |
| --- | --- | --- | --- | --- |
| `ASUS_TUF_M3_SET_SCROL_SENS` | Sets scroll wheel sensitivity | `int*` (sensitivity) | 0–100 | 1 |
| `ASUS_TUF_M3_GET_SCROL_SENS` | Gets scroll wheel sensitivity | `int*` (sensitivity) | 0–100 | 1 |
| `ASUS_TUF_M3_SET_MOV_SENS` | Sets cursor movement sensitivity | `int` (sensitivity) | 0–100 | 1 |
| `ASUS_TUF_M3_GET_MOV_SENS` | Gets cursor movement sensitivity | `int*` (sensitivity) | 0–100 | 1 |
| `ASUS_TUF_M3_DISABLE_LEFT` | Disables left button | None | None | Enabled |
| `ASUS_TUF_M3_ENABLE_LEFT` | Enables left button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_RIGHT` | Disables right button | None | None | Enabled |
| `ASUS_TUF_M3_ENABLE_RIGHT` | Enables right button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_MID` | Disables middle button | None | None | Enabled |
| `ASUS_TUF_M3_ENABLE_MID` | Enables middle button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_FORW` | Disables forward button | None | None | Enabled |
| `ASUS_TUF_M3_ENABLE_FORW` | Enables forward button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_BACK` | Disables backward button | None | None | Enabled |
| `ASUS_TUF_M3_ENABLE_BACK` | Enables backward button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_SCROL` | Disables scroll wheel | None | None | Enabled |
| `ASUS_TUF_M3_ENABLE_SCROL` | Enables scroll wheel | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_X` | Disables X-axis movement | None | None | Enabled |
| `ASUS_TUF_M3_ENABLE_X` | Enables X-axis movement | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_Y` | Disables Y-axis movement | None | None | Enabled |
| `ASUS_TUF_M3_ENABLE_Y` | Enables Y-axis movement | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_ALL` | Disables all buttons and movement | None | None | Enabled |
| `ASUS_TUF_M3_ENABLE_ALL` | Enables all buttons and movement | None | None | Enabled |

### Notes

- **Sensitivity**: Values range from 0 (minimum sensitivity, no response) to 100 (maximum sensitivity, fastest response).
- **Button/Axis State**: Disabling a button or axis suppresses corresponding input events. For example, `ASUS_TUF_M3_DISABLE_LEFT` prevents left-click events.
- **Error Handling**: Invalid sensitivity values (&lt;0 or &gt;100) return `EINVAL`. Unrecognized commands return `ENOTTY`.

### Example

Set scroll sensitivity to 50:

```c
int fd = open("/dev/asus_tuf_m3", O_RDWR);
int value = 50;
ioctl(fd, ASUS_TUF_M3_SET_SCROL_SENS, &value);
close(fd);
```

## Test Program

The test program (`mouse_test.c`) demonstrates `ioctl` usage by enabling all inputs, reading the current scroll sensitivity, setting it to 10, and verifying the change.

### Compilation

```bash
gcc -o test mouse_test.c
```

### Execution

```bash
sudo ./test
```

### Expected Output

```
Current scroll sensitivity is 1
Current scroll sensitivity is 10
```

### Troubleshooting

- **Permission denied**: Ensure the program runs with `sudo` or adjust `/dev/asus_tuf_m3` permissions.
- **Device not found**: Verify the driver is loaded (`lsmod | grep asus_tuf_m3`) and the device file exists.
- **IOCTL errors**: Check `errno` for details (e.g., `EINVAL` for invalid sensitivity).

## Usage Examples

### Disabling a Stuck Button

To disable a malfunctioning right button:

```c
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>

#define DEVICE_PATH "/dev/asus_tuf_m3"
#define ASUS_TUF_M3_DISABLE_RIGHT _IOR('m', 7, int)
#define ASUS_TUF_M3_ENABLE_RIGHT _IOR('m', 8, int)

int main() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return errno;
    }
    if (ioctl(fd, ASUS_TUF_M3_DISABLE_RIGHT) < 0) {
        perror("Failed to disable right button");
        close(fd);
        return errno;
    }
    printf("Right button disabled\n");
    close(fd);
    return 0;
}
```

### Adjusting Sensitivity

To set cursor movement sensitivity to 75:

```bash
gcc -o set_sensitivity set_sensitivity.c
sudo ./set_sensitivity
```

```c
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>

#define DEVICE_PATH "/dev/asus_tuf_m3"
#define ASUS_TUF_M3_SET_MOV_SENS _IOW('m', 3, int)

int main() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return errno;
    }
    int value = 75;
    if (ioctl(fd, ASUS_TUF_M3_SET_MOV_SENS, &value) < 0) {
        perror("Failed to set movement sensitivity");
        close(fd);
        return errno;
    }
    printf("Movement sensitivity set to 75\n");
    close(fd);
    return 0;
}
```

## Limitations and Known Issues

- **RGB Control**: Not supported due to proprietary firmware limitations.
- **Rapid Sensitivity Changes**: Changing sensitivity rapidly (e.g., in a loop) may cause temporary unresponsiveness. Workaround: Add a 10ms delay between `ioctl` calls.
- **Kernel Compatibility**: Tested only on kernel 6.11.0-25-generic. Older kernels may require patches.
- **No DPI Control**: The driver does not support hardware DPI settings, only relative sensitivity adjustments.

Future Improvements

- Consolidate enable/disable commands into single `ioctl` commands with a 0–1 argument to reduce the number of commands.
- Add support for persistent configuration via module parameters or a configuration file.
- Implement RGB control if firmware documentation becomes available.

## Maintenance and Support

- **Repository**: Source code is available at `<repository-url>` (e.g., GitHub).
- **Bug Reports**: Submit issues via the repository’s issue tracker.
- **Contact**: Email `<support-email>` for inquiries.

## License

This driver is licensed under the GNU General Public License v2.0 (GPL-2.0). See the `LICENSE` file in the repository for details.

## Changelog

- **v1.0.0** (2025-05-12):
  - Initial release with HID-based driver and `ioctl` interface.
  - Supports button/axis control and sensitivity adjustments.
