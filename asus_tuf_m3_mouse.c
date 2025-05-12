#include <linux/hid.h>
#include <linux/module.h>
#include <linux/usb.h>

#define CLASS_NAME "asus_tuf_m3_character"
#define DEVICE_NAME "asus_tuf_m3"

#define DRIVER_NAME "asus_tuf_m3_mouse"
#define VENDOR_ID  0x0B05
#define PRODUCT_ID 0x1910

struct asus_tuf_m3 {
    struct input_dev *input;
};
int dpi = 1;
int scroll_sens = 1;
int basic_control = 0xFF;

/* region character device driver */

/* Ioctl commands */
#define ASUS_TUF_M3_IOC_MAGIC 'm'
#define ASUS_TUF_M3_SET_SENSITIVITY _IOW(ASUS_TUF_M3_IOC_MAGIC, 1, int)
#define ASUS_TUF_M3_GET_SENSITIVITY _IOR(ASUS_TUF_M3_IOC_MAGIC, 2, int)
#define ASUS_TUF_M3_SET_DPI         _IOW(ASUS_TUF_M3_IOC_MAGIC, 3, int)
#define ASUS_TUF_M3_GET_DPI         _IOR(ASUS_TUF_M3_IOC_MAGIC, 4, int)
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

static struct class* asus_tuf_m3_class  = NULL;
static struct device* asus_tuf_m3_dev   = NULL;
static int major_number;
static bool class_created = false;
static DEFINE_MUTEX(class_mutex);

// in/out controll function
static long asus_tuf_m3_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
        // Clicking click
        case ASUS_TUF_M3_DISABLE_ALL:
            basic_control &= ~0xFF;
            break;
        case ASUS_TUF_M3_ENABLE_ALL:
            basic_control |= 0xFF;
            break;

        case ASUS_TUF_M3_DISABLE_LEFT:
            basic_control &= ~0x01;
            break;
        case ASUS_TUF_M3_ENABLE_LEFT:
            basic_control |= 0x01;
            break;
        case ASUS_TUF_M3_DISABLE_RIGHT:
            basic_control &= ~0x02;
            break;
        case ASUS_TUF_M3_ENABLE_RIGHT:
            basic_control |= 0x02;
            break;
        case ASUS_TUF_M3_DISABLE_MID:
            basic_control &= ~0x04;
            break;
        case ASUS_TUF_M3_ENABLE_MID:
            basic_control |= 0x04;
            break;
        case ASUS_TUF_M3_DISABLE_BACK:
            basic_control &= ~0x08;
            break;
        case ASUS_TUF_M3_ENABLE_BACK:
            basic_control |= 0x08;
            break;
        case ASUS_TUF_M3_DISABLE_FORW:
            basic_control &= ~0x10;
            break;
        case ASUS_TUF_M3_ENABLE_FORW:
            basic_control |= 0x10;
            break;
        
        
        default:
            break;
    }
    return 0;
}

// register function
static int asus_tuf_m3_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "asus_tuf_m3: character driver part opened\n");
    return 0;
}

// unregister function
static int asus_tuf_m3_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "asus_tuf_m3: character driver part released\n");
    return 0;
}

// construct file operations
static const struct file_operations asus_tuf_m3_fops = {
    .open = asus_tuf_m3_open,
    .release = asus_tuf_m3_release,
    .unlocked_ioctl = asus_tuf_m3_ioctl,
};

static int register_character_driver(void)
{
    if (class_created)
        return -EBUSY;

    // Create a char device
    major_number = register_chrdev(0, DEVICE_NAME, &asus_tuf_m3_fops);
    if (major_number < 0) {
        printk(KERN_ERR "asus_tuf_m3: Failed to register a major number\n");
        return major_number;
    }

    asus_tuf_m3_class = class_create(CLASS_NAME);
    if (IS_ERR(asus_tuf_m3_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "asus_tuf_m3: Failed to register device class\n");
        return PTR_ERR(asus_tuf_m3_class);
    }

    asus_tuf_m3_dev = device_create(asus_tuf_m3_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(asus_tuf_m3_dev)) {
        class_destroy(asus_tuf_m3_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "asus_tuf_m3: Failed to create the device\n");
        return PTR_ERR(asus_tuf_m3_dev);
    }
    class_created = true;
    return 0;
}

static void deregister_character_driver(void)
{
    class_created = false;
    device_destroy(asus_tuf_m3_class, MKDEV(major_number, 0));
    class_unregister(asus_tuf_m3_class);
    class_destroy(asus_tuf_m3_class);
    unregister_chrdev(major_number, DEVICE_NAME);
}

/* endregion character device driver */

/* region HID client driver */

// create device table for hid driver
static const struct hid_device_id asus_tuf_m3_id_table[] = {
    { HID_USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
    { }
};
MODULE_DEVICE_TABLE(hid, asus_tuf_m3_id_table);

// // log information to the kernel (temporary) for debug
// static int asus_tuf_m3_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
// {
//     if (size == 8) {
//         printk_ratelimited(KERN_INFO "ASUS TUF's HID report: %02x %02x %02x %02x %02x %02x %02x %02x\n",
//                            data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
//     }
//     return 0;
// }

// handle event for mouse
static int asus_tuf_m3_event(struct hid_device *hdev, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
    struct asus_tuf_m3 *mouse = hid_get_drvdata(hdev);
    struct input_dev *input = mouse->input;

    switch (usage->hid) {
        // Button case
        case HID_UP_BUTTON | 0x0001:
            if ((basic_control & 0x01) != 0)     // left click allowed
                input_report_key(input, BTN_LEFT, value);
            break;
        case HID_UP_BUTTON | 0x0002:
            if ((basic_control & 0x02) != 0)     // right click allowed
                input_report_key(input, BTN_RIGHT, value);
            break;
        case HID_UP_BUTTON | 0x0003:
            if ((basic_control & 0x04) != 0)     // middle click allowed
                input_report_key(input, BTN_MIDDLE, value);
            break;
        case HID_UP_BUTTON | 0x0004:
            if ((basic_control & 0x08) != 0)     // back click click allowed
                input_report_key(input, BTN_BACK, value);
            break;
        case HID_UP_BUTTON | 0x0005:
            if ((basic_control & 0x10) != 0)     // forward click click allowed
                input_report_key(input, BTN_FORWARD, value);
            break;

        // Move
        case HID_GD_X:
            input_report_rel(input, REL_X, value);
            break;
        case HID_GD_Y:
            input_report_rel(input, REL_Y, value);
            break;

        // Scroll
        case HID_GD_WHEEL:
            input_report_rel(input, REL_WHEEL, value);
            break;
        
    }

    input_sync(input);
    return 1;
}

// register driver
static int asus_tuf_m3_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    struct asus_tuf_m3 *mouse;
    struct input_dev *input_dev;
    int ret;

    printk(KERN_INFO "asus_tuf_m3: find the model!!! ASUS TUF M3 (Vendor: 0x%04x, Product: 0x%04x)\n",
           id->vendor, id->product);

    /* Allocate memory for the mouse structure */
    mouse = kzalloc(sizeof(*mouse), GFP_KERNEL);
    if (!mouse) {
        hid_err(hdev, "Failed to allocate mouse structure\n");
        return -ENOMEM;
    }

    /* Allocate input device */
    input_dev = input_allocate_device();
    if (!input_dev) {
        hid_err(hdev, "Failed to allocate input device\n");
        kfree(mouse);
        return -ENOMEM;
    }

    /* Associate mouse structure with HID device */
    mouse->input = input_dev;
    hid_set_drvdata(hdev, mouse);

    /* Initialize input device */
    input_dev->name = "ASUS TUF M3 Mouse";
    input_dev->phys = DRIVER_NAME "/input0";
    input_dev->dev.parent = &hdev->dev;
    input_dev->id.bustype = BUS_USB;
    input_dev->id.vendor = id->vendor;
    input_dev->id.product = id->product;
    input_dev->id.version = 0x0100;

    /* Set supported events */
    set_bit(EV_KEY, input_dev->evbit); /* Key events (buttons) */
    set_bit(BTN_LEFT, input_dev->keybit);
    set_bit(BTN_RIGHT, input_dev->keybit);
    set_bit(BTN_MIDDLE, input_dev->keybit);
    set_bit(BTN_FORWARD, input_dev->keybit);
    set_bit(BTN_BACK, input_dev->keybit);

    set_bit(EV_REL, input_dev->evbit); /* Relative movement events */
    set_bit(REL_X, input_dev->relbit);
    set_bit(REL_Y, input_dev->relbit);
    set_bit(REL_WHEEL, input_dev->relbit); /* Scroll wheel */

    /* Parse HID report descriptor */
    ret = hid_parse(hdev);
    if (ret) {
        hid_err(hdev, "Failed to parse HID report descriptor: %d\n", ret);
        goto err_free_input;
    }

    /* Start HID device (enable event processing) */
    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret) {
        hid_err(hdev, "Failed to start HID device: %d\n", ret);
        goto err_free_input;
    }

    /* Register input device */
    ret = input_register_device(input_dev);
    if (ret) {
        hid_err(hdev, "Failed to register input device: %d\n", ret);
        goto err_stop_hw;
    }

    hid_info(hdev, "ASUS TUF M3 connected\n");

    /* CHARACTER DEVICE INITIALIZE */

    mutex_lock(&class_mutex);
    if (!class_created) {
        /* Create character device class */
        register_character_driver();
        class_created = true;
        hid_info(hdev, "Device class created\n");
    }
    mutex_unlock(&class_mutex);

    /* CHARACTER DEVICE INITIALIZE */

    return 0;

err_stop_hw:
    hid_hw_stop(hdev);
err_free_input:
    input_free_device(input_dev);
    kfree(mouse);
    hid_set_drvdata(hdev, NULL);
    return ret;
}

// remove driver from HID drvier
static void asus_tuf_m3_remove(struct hid_device *hdev)
{
    struct asus_tuf_m3 *mouse = hid_get_drvdata(hdev);

    if (!mouse)
        return;

    hid_hw_stop(hdev);
    input_unregister_device(mouse->input);
    kfree(mouse);
    hid_set_drvdata(hdev, NULL);
    hid_info(hdev, "ASUS TUF M3 disconnected\n");
    
    class_created = false;
    deregister_character_driver();  // remove the character driver
}

// construct the driver
static struct hid_driver asus_tuf_m3_driver = {
    .name = DRIVER_NAME,
    .id_table = asus_tuf_m3_id_table,
    .probe = asus_tuf_m3_probe,
    .remove = asus_tuf_m3_remove,
    // .raw_event = asus_tuf_m3_raw_event,
    .event = asus_tuf_m3_event,
};

/* endregion HID client driver */

static int __init asus_tuf_m3_init(void)
{
    int ret;
    /* Register HID driver */
    ret = hid_register_driver(&asus_tuf_m3_driver);
    if (ret) {
        pr_err("asus_tuf_m3: Failed to register a HID driver: %d\n", ret);
    }

    return ret;
}

static void __exit asus_tuf_m3_exit(void)
{
    printk(KERN_INFO "asus_tuf_m3: driver is unregistering from kernel\n");
    hid_unregister_driver(&asus_tuf_m3_driver);
    printk(KERN_INFO "asus_tuf_m3: driver is unregistered\n");
}

module_init(asus_tuf_m3_init);
module_exit(asus_tuf_m3_exit);

MODULE_AUTHOR("LOI");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("USB MOUSE Driver");