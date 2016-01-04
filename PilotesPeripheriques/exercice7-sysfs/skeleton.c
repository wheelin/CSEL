#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include "linux/device.h"
#include "linux/platform_device.h"

static char sysfs_buf[1000];

static ssize_t skeleton_show_buf(struct device * dev,
	struct device_attribute *attr, char *buf)
{
	strcpy(buf, sysfs_buf);
	return strlen(sysfs_buf);
}

static ssize_t skeleton_store_buf(struct device * dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int len = sizeof(sysfs_buf) - 1;
	if(len > count) len = count;
	strncpy(sysfs_buf, buf, len);
	sysfs_buf[len] = 0;
	return len;
}

DEVICE_ATTR(attr, 0666, skeleton_show_buf, skeleton_store_buf);

static void sysfs_dev_release(struct device *dev)
{

}

static struct platform_driver sysfs_driver = {
	.driver = {.name = "skeleton",},
};

static struct platform_device sysfs_device = {
	.name = "skeleton",
	.id = -1,
	.dev.release = sysfs_dev_release,
};

static int __init skeleton_init(void)
{
	int status = 0;
	if(status == 0)
		status = platform_driver_register(&sysfs_driver);
	if(status == 0)
		status = platform_device_register(&sysfs_device);
	if(status == 0)
		status = device_create_file(&sysfs_device.dev, &dev_attr_attr);

	pr_info("Linux module skeleton loaded\n");
	return status;
}

static void __exit skeleton_exit(void)
{
	device_remove_file(&sysfs_device.dev, &dev_attr_attr);
	platform_device_unregister(&sysfs_device);
	platform_driver_unregister(&sysfs_driver);
	pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Greg");
MODULE_DESCRIPTION("Sysfs character oriented driver");
MODULE_LICENSE("GPL");
