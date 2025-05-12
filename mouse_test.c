#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h> // Include errno header

#define DEVICE_PATH "/dev/asus_tuf_m3"
/* Ioctl commands */
#define ASUS_TUF_M3_IOC_MAGIC 'm'
#define ASUS_TUF_M3_SET_SCROL_SENS  _IOW(ASUS_TUF_M3_IOC_MAGIC, 1, int)
#define ASUS_TUF_M3_GET_SCROL_SENS  _IOR(ASUS_TUF_M3_IOC_MAGIC, 2, int)
#define ASUS_TUF_M3_SET_MOV_SENS    _IOW(ASUS_TUF_M3_IOC_MAGIC, 3, int)
#define ASUS_TUF_M3_GET_MOV_SENS    _IOR(ASUS_TUF_M3_IOC_MAGIC, 4, int)
// control over basic function
#define ASUS_TUF_M3_DISABLE_LEFT    _IOR(ASUS_TUF_M3_IOC_MAGIC, 5, int)
#define ASUS_TUF_M3_ENABLE_LEFT     _IOR(ASUS_TUF_M3_IOC_MAGIC, 6, int)
#define ASUS_TUF_M3_DISABLE_RIGHT   _IOR(ASUS_TUF_M3_IOC_MAGIC, 7, int)
#define ASUS_TUF_M3_ENABLE_RIGHT    _IOR(ASUS_TUF_M3_IOC_MAGIC, 8, int)
#define ASUS_TUF_M3_DISABLE_MID     _IOR(ASUS_TUF_M3_IOC_MAGIC, 9, int)
#define ASUS_TUF_M3_ENABLE_MID      _IOR(ASUS_TUF_M3_IOC_MAGIC, 10, int)
#define ASUS_TUF_M3_DISABLE_FORW    _IOR(ASUS_TUF_M3_IOC_MAGIC, 11, int)
#define ASUS_TUF_M3_ENABLE_FORW     _IOR(ASUS_TUF_M3_IOC_MAGIC, 12, int)
#define ASUS_TUF_M3_DISABLE_BACK    _IOR(ASUS_TUF_M3_IOC_MAGIC, 13, int)
#define ASUS_TUF_M3_ENABLE_BACK     _IOR(ASUS_TUF_M3_IOC_MAGIC, 14, int)
#define ASUS_TUF_M3_DISABLE_SCROL   _IOR(ASUS_TUF_M3_IOC_MAGIC, 15, int)
#define ASUS_TUF_M3_ENABLE_SCROL    _IOR(ASUS_TUF_M3_IOC_MAGIC, 16, int)
#define ASUS_TUF_M3_DISABLE_X       _IOR(ASUS_TUF_M3_IOC_MAGIC, 17, int)
#define ASUS_TUF_M3_ENABLE_X        _IOR(ASUS_TUF_M3_IOC_MAGIC, 18, int)
#define ASUS_TUF_M3_DISABLE_Y       _IOR(ASUS_TUF_M3_IOC_MAGIC, 19, int)
#define ASUS_TUF_M3_ENABLE_Y        _IOR(ASUS_TUF_M3_IOC_MAGIC, 20, int)
#define ASUS_TUF_M3_DISABLE_ALL     _IOR(ASUS_TUF_M3_IOC_MAGIC, 21, int)
#define ASUS_TUF_M3_ENABLE_ALL      _IOR(ASUS_TUF_M3_IOC_MAGIC, 22, int)

int main() {
    int fd;
    int data;

    // Open the device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }
    // Disable click
    if (ioctl(fd, ASUS_TUF_M3_DISABLE_ALL) < 0) {
        perror("Failed to exicute control command");
        close(fd);
        return errno;
    }
    // play around with mov sens or scroll sens
    if (ioctl(fd, ASUS_TUF_M3_GET_SCROL_SENS, &data) < 0) {
        perror("Failed to read moving sensitivity");
        close(fd);
        return errno;
    }
    printf("current moving sensitivity is %d\n", data);
    
    data = 10;
    if (ioctl(fd, ASUS_TUF_M3_SET_MOV_SENS, &data) < 0) {
        perror("Failed to read moving sensitivity");
        close(fd);
        return errno;
    }

    if (ioctl(fd, ASUS_TUF_M3_GET_MOV_SENS, &data) < 0) {
        perror("Failed to read moving sensitivity");
        close(fd);
        return errno;
    }
    printf("current moving sensitivity is %d\n", data);
    // Close the device
    close(fd);
    return 0;
}