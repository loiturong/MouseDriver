# ASUS TUF M3 Mouse Driver

## Overview

The `asus_tuf_m3` driver is a custom HID-based kernel driver for the ASUS TUF Gaming M3 mouse. It replaces the generic Linux HID driver (`hid-generic`) to provide fine-grained control over mouse functionality via a character device interface. Key features include enabling/disabling individual buttons, scroll wheel, and movement axes, as well as adjusting scroll and cursor movement sensitivity.

### Supported Hardware

This driver is designed specifically for the ASUS TUF Gaming M3 mouse (USB VID: 0x0B05, PID: 0x1910). Compatibility with other ASUS mice or firmware versions is not guaranteed.

### Advantages

- Lightweight alternative to ASUS's proprietary drivers.
- Programmatic control over button and axis functionality.
- Adjustable sensitivity settings directly from the kernel.

## Installation and Setup

### Kernel Requirements

- Tested on Linux kernel version 6.11.0-25-generic. Other versions may work but are untested.
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

If the generic HID driver (`hid-generic`) is already bound to the mouse, unload it first:

```bash
sudo rmmod hid-generic
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

To restore generic HID functionality:

```bash
sudo modprobe hid-generic
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

The SIMPLE driver exposes a character device (`/dev/asus_tuf_m3`) that supports the following `ioctl` commands. All commands return 0 on success or -1 on error, setting `errno` to `EFAULT` for user-space copy failures.

| Command | Purpose | Input/Output | Valid Range | Default |
| --- | --- | --- | --- | --- |
| `ASUS_TUF_M3_SET_SCROL_SENS` | Sets scroll wheel sensitivity multiplier | `int` (multiplier) | Any `int` (0 disables, 1 is normal, >1 amplifies) | 1 |
| `ASUS_TUF_M3_GET_SCROL_SENS` | Gets scroll wheel sensitivity multiplier | `int*` (multiplier) | Any `int` | 1 |
| `ASUS_TUF_M3_SET_MOV_SENS` | Sets cursor movement sensitivity multiplier | `int` (multiplier) | Any `int` (0 disables, 1 is normal, >1 amplifies) | 1 |
| `ASUS_TUF_M3_GET_MOV_SENS` | Gets cursor movement sensitivity multiplier | `int*` (multiplier) | Any `int` | 1 |
| `ASUS_TUF_M3_DISABLE_LEFT` | Disables left button | None | None | Disabled |
| `ASUS_TUF_M3_ENABLE_LEFT` | Enables left button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_RIGHT` | Disables right button | None | None | Disabled |
| `ASUS_TUF_M3_ENABLE_RIGHT` | Enables right button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_MID` | Disables middle button | None | None | Disabled |
| `ASUS_TUF_M3_ENABLE_MID` | Enables middle button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_FORW` | Disables forward button | None | None | Disabled |
| `ASUS_TUF_M3_ENABLE_FORW` | Enables forward button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_BACK` | Disables backward button | None | None | Disabled |
| `ASUS_TUF_M3_ENABLE_BACK` | Enables backward button | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_SCROL` | Disables scroll wheel | None | None | Disabled |
| `ASUS_TUF_M3_ENABLE_SCROL` | Enables scroll wheel | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_X` | Disables X-axis movement | None | None | Disabled |
| `ASUS_TUF_M3_ENABLE_X` | Enables X-axis movement | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_Y` | Disables Y-axis movement | None | None | Disabled |
| `ASUS_TUF_M3_ENABLE_Y` | Enables Y-axis movement | None | None | Enabled |
| `ASUS_TUF_M3_DISABLE_ALL` | Disables all buttons and movement | None | None | Disabled |
| `ASUS_TUF_M3_ENABLE_ALL` | Enables all buttons and movement | None | None | Enabled |

### Notes

- **Sensitivity**: Values are multipliers applied to movement (`REL_X`, `REL_Y`) or scroll (`REL_WHEEL`). A value of 0 disables the feature, 1 is the default (normal), and values >1 amplify the effect. Negative values are accepted but may cause unpredictable behavior.
- **Button/Axis State**: Disabling a button or axis suppresses corresponding input events. For example, `ASUS_TUF_M3_DISABLE_LEFT` prevents left-click events.
- **Error Handling**: Commands return `-EFAULT` if copying to/from user space fails. Unrecognized commands are ignored (return 0).

### Example

Set scroll sensitivity to 2 (double the normal scroll speed):

```c
int fd = open("/dev/asus_tuf_m3", O_RDWR);
int value = 2;
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
- **IOCTL errors**: Check `errno` for details (e.g., `EFAULT` for copy failures).

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

To set cursor movement sensitivity to 2 (double speed):

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
    int value = 2;
    if (ioctl(fd, ASUS_TUF_M3_SET_MOV_SENS, &value) < 0) {
        perror("Failed to set movement sensitivity");
        close(fd);
        return errno;
    }
    printf("Movement sensitivity set to 2\n");
    close(fd);
    return 0;
}
```

## Limitations and Known Issues

- **RGB Control**: Not supported due to proprietary firmware limitations.
- **Sensitivity Validation**: Sensitivity values are not bounds-checked, allowing negative or extreme values that may cause unpredictable behavior.
- **Kernel Compatibility**: Tested only on kernel 6.11.0-25-generic. Older kernels may require patches.
- **No DPI Control**: The driver does not support hardware DPI settings, only relative sensitivity adjustments.
- **Untested Edge Cases**: Rapid `ioctl` calls or extreme sensitivity values may lead to unexpected behavior (untested).

### Future Improvements

- Add sensitivity validation (e.g., enforce 0–100 range) to prevent extreme values.
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
