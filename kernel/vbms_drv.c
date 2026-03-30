#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "vbms_ioctl.h"

#define DEVICE_NAME "vbms0"
#define CLASS_NAME  "vbms"

#define VBMS_CTRL_ENABLE_MEASUREMENT      0
#define VBMS_CTRL_ENABLE_FAULT_DETECTION  1

#define VBMS_STATUS_DEVICE_READY          0
#define VBMS_STATUS_MEASUREMENT_VALID     1
#define VBMS_STATUS_FAULT_ACTIVE          2

#define VBMS_FAULT_OVERVOLTAGE            0
#define VBMS_FAULT_OVERTEMPERATURE        1

struct vbms_dev {
    struct mutex lock;
    struct cdev cdev;
    dev_t dev_num;
    struct class *class;
    struct device *device;

    u32 control;
    u32 status;
    u32 fault;
    u32 voltage_mv;
    u32 temperature_cx10;
    u32 overvoltage_mv;
    u32 overtemperature_cx10;
};

static struct vbms_dev g_vbms;

static inline void vbms_set_bit(u32 *value, u8 bit)
{
    *value |= (1U << bit);
}

static inline void vbms_clear_bit(u32 *value, u8 bit)
{
    *value &= ~(1U << bit);
}

static inline bool vbms_is_bit_set(u32 value, u8 bit)
{
    return (value & (1U << bit)) != 0U;
}

static void vbms_evaluate_faults(struct vbms_dev *dev)
{
    dev->fault = 0U;

    if (!vbms_is_bit_set(dev->control, VBMS_CTRL_ENABLE_FAULT_DETECTION)) {
        vbms_clear_bit(&dev->status, VBMS_STATUS_FAULT_ACTIVE);
        return;
    }

    if (dev->voltage_mv > dev->overvoltage_mv)
        vbms_set_bit(&dev->fault, VBMS_FAULT_OVERVOLTAGE);

    if (dev->temperature_cx10 > dev->overtemperature_cx10)
        vbms_set_bit(&dev->fault, VBMS_FAULT_OVERTEMPERATURE);

    if (dev->fault != 0U)
        vbms_set_bit(&dev->status, VBMS_STATUS_FAULT_ACTIVE);
    else
        vbms_clear_bit(&dev->status, VBMS_STATUS_FAULT_ACTIVE);
}

static int vbms_open(struct inode *inode, struct file *file)
{
    file->private_data = &g_vbms;
    pr_info("vbms: device opened\n");
    return 0;
}

static int vbms_release(struct inode *inode, struct file *file)
{
    pr_info("vbms: device closed\n");
    return 0;
}

static ssize_t vbms_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct vbms_dev *dev = file->private_data;
    char kbuf[256];
    int len;
    int ret;

    mutex_lock(&dev->lock);

    len = scnprintf(
        kbuf,
        sizeof(kbuf),
        "control=%u status=%u fault=%u voltage_mv=%u temperature_cx10=%u ov_threshold_mv=%u ot_threshold_cx10=%u\n",
        dev->control,
        dev->status,
        dev->fault,
        dev->voltage_mv,
        dev->temperature_cx10,
        dev->overvoltage_mv,
        dev->overtemperature_cx10
    );

    mutex_unlock(&dev->lock);

    ret = simple_read_from_buffer(buf, count, ppos, kbuf, len);
    return ret;
}

static ssize_t vbms_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct vbms_dev *dev = file->private_data;
    char kbuf[64];
    u32 voltage_mv;
    u32 temperature_cx10;

    if (count >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    kbuf[count] = '\0';

    /*
     * Expected format:
     *   "<voltage_mv> <temperature_cx10>"
     * Example:
     *   "4200 350"
     */
    if (sscanf(kbuf, "%u %u", &voltage_mv, &temperature_cx10) != 2)
        return -EINVAL;

    mutex_lock(&dev->lock);

    if (vbms_is_bit_set(dev->control, VBMS_CTRL_ENABLE_MEASUREMENT)) {
        dev->voltage_mv = voltage_mv;
        dev->temperature_cx10 = temperature_cx10;
        vbms_set_bit(&dev->status, VBMS_STATUS_MEASUREMENT_VALID);
        vbms_evaluate_faults(dev);
    }

    mutex_unlock(&dev->lock);

    return count;
}

static long vbms_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct vbms_dev *dev = file->private_data;
    struct vbms_thresholds thresholds;
    struct vbms_measurement measurement;
    struct vbms_status status;
    u32 enable;

    mutex_lock(&dev->lock);

    switch (cmd) {
    case VBMS_IOCTL_SET_THRESHOLDS:
        if (copy_from_user(&thresholds, (void __user *)arg, sizeof(thresholds))) {
            mutex_unlock(&dev->lock);
            return -EFAULT;
        }

        dev->overvoltage_mv = thresholds.overvoltage_mv;
        dev->overtemperature_cx10 = thresholds.overtemperature_cx10;
        vbms_evaluate_faults(dev);
        break;

    case VBMS_IOCTL_GET_THRESHOLDS:
        thresholds.overvoltage_mv = dev->overvoltage_mv;
        thresholds.overtemperature_cx10 = dev->overtemperature_cx10;

        if (copy_to_user((void __user *)arg, &thresholds, sizeof(thresholds))) {
            mutex_unlock(&dev->lock);
            return -EFAULT;
        }
        break;

    case VBMS_IOCTL_PUSH_MEASUREMENT:
        if (copy_from_user(&measurement, (void __user *)arg, sizeof(measurement))) {
            mutex_unlock(&dev->lock);
            return -EFAULT;
        }

        if (vbms_is_bit_set(dev->control, VBMS_CTRL_ENABLE_MEASUREMENT)) {
            dev->voltage_mv = measurement.voltage_mv;
            dev->temperature_cx10 = measurement.temperature_cx10;
            vbms_set_bit(&dev->status, VBMS_STATUS_MEASUREMENT_VALID);
            vbms_evaluate_faults(dev);
        }
        break;

    case VBMS_IOCTL_GET_STATUS:
        status.control = dev->control;
        status.status = dev->status;
        status.fault = dev->fault;
        status.voltage_mv = dev->voltage_mv;
        status.temperature_cx10 = dev->temperature_cx10;
        status.overvoltage_mv = dev->overvoltage_mv;
        status.overtemperature_cx10 = dev->overtemperature_cx10;

        if (copy_to_user((void __user *)arg, &status, sizeof(status))) {
            mutex_unlock(&dev->lock);
            return -EFAULT;
        }
        break;

    case VBMS_IOCTL_CLEAR_FAULTS:
        dev->fault = 0U;
        vbms_clear_bit(&dev->status, VBMS_STATUS_FAULT_ACTIVE);
        vbms_evaluate_faults(dev);
        break;

    case VBMS_IOCTL_ENABLE_MEAS:
        if (copy_from_user(&enable, (void __user *)arg, sizeof(enable))) {
            mutex_unlock(&dev->lock);
            return -EFAULT;
        }

        if (enable)
            vbms_set_bit(&dev->control, VBMS_CTRL_ENABLE_MEASUREMENT);
        else
            vbms_clear_bit(&dev->control, VBMS_CTRL_ENABLE_MEASUREMENT);
        break;

    case VBMS_IOCTL_ENABLE_FAULTS:
        if (copy_from_user(&enable, (void __user *)arg, sizeof(enable))) {
            mutex_unlock(&dev->lock);
            return -EFAULT;
        }

        if (enable)
            vbms_set_bit(&dev->control, VBMS_CTRL_ENABLE_FAULT_DETECTION);
        else
            vbms_clear_bit(&dev->control, VBMS_CTRL_ENABLE_FAULT_DETECTION);

        vbms_evaluate_faults(dev);
        break;

    default:
        mutex_unlock(&dev->lock);
        return -ENOTTY;
    }

    mutex_unlock(&dev->lock);
    return 0;
}

static const struct file_operations vbms_fops = {
    .owner = THIS_MODULE,
    .open = vbms_open,
    .release = vbms_release,
    .read = vbms_read,
    .write = vbms_write,
    .unlocked_ioctl = vbms_ioctl,
    .llseek = no_llseek,
};

static int __init vbms_init(void)
{
    int ret;

    memset(&g_vbms, 0, sizeof(g_vbms));
    mutex_init(&g_vbms.lock);

    g_vbms.overvoltage_mv = 4200U;
    g_vbms.overtemperature_cx10 = 600U;
    vbms_set_bit(&g_vbms.status, VBMS_STATUS_DEVICE_READY);

    ret = alloc_chrdev_region(&g_vbms.dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("vbms: alloc_chrdev_region failed\n");
        return ret;
    }

    cdev_init(&g_vbms.cdev, &vbms_fops);
    g_vbms.cdev.owner = THIS_MODULE;

    ret = cdev_add(&g_vbms.cdev, g_vbms.dev_num, 1);
    if (ret < 0) {
        pr_err("vbms: cdev_add failed\n");
        unregister_chrdev_region(g_vbms.dev_num, 1);
        return ret;
    }

    g_vbms.class = class_create(CLASS_NAME);
    if (IS_ERR(g_vbms.class)) {
        pr_err("vbms: class_create failed\n");
        cdev_del(&g_vbms.cdev);
        unregister_chrdev_region(g_vbms.dev_num, 1);
        return PTR_ERR(g_vbms.class);
    }

    g_vbms.device = device_create(g_vbms.class, NULL, g_vbms.dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(g_vbms.device)) {
        pr_err("vbms: device_create failed\n");
        class_destroy(g_vbms.class);
        cdev_del(&g_vbms.cdev);
        unregister_chrdev_region(g_vbms.dev_num, 1);
        return PTR_ERR(g_vbms.device);
    }

    pr_info("vbms: module loaded, device /dev/%s created\n", DEVICE_NAME);
    return 0;
}

static void __exit vbms_exit(void)
{
    device_destroy(g_vbms.class, g_vbms.dev_num);
    class_destroy(g_vbms.class);
    cdev_del(&g_vbms.cdev);
    unregister_chrdev_region(g_vbms.dev_num, 1);
    pr_info("vbms: module unloaded\n");
}

module_init(vbms_init);
module_exit(vbms_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charan H S");
MODULE_DESCRIPTION("Virtual BMS kernel module with char device and ioctl interface");
MODULE_VERSION("1.0");
