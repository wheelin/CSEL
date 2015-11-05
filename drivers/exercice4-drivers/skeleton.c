#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include "linux/fs.h"
#include "linux/kdev_t.h"
#include "linux/cdev.h"
#include "linux/uaccess.h"

static int major_version;
static int minor_version;
static dev_t device;

static struct cdev mod_cdev;

static char** gBuf;
static int* count;

static int num_devs=2;
module_param(num_devs,int,0);

static ssize_t mod_open(struct inode * i, struct file * f)
{
	pr_info("skeleton : open operation, major version : %d, minor version : %d\n", imajor(i), iminor(i));
	if((f->f_mode & (FMODE_READ | FMODE_WRITE)) != 0)
		pr_info("skeleton : opened for reading and writing...\n");
	else if ((f->f_mode & (FMODE_READ)) != 0)
		pr_info("skeleton : opened for reading only\n");
	else if ((f->f_mode & (FMODE_WRITE)) != 0)
		pr_info("skeleton : opened for writing only\n");

	return 0;
}

static ssize_t mod_read(struct file * f, 
	char __user *buf, size_t size, loff_t * off)
{
	int minor = MINOR(f->f_dentry->d_inode->i_rdev);
	ssize_t remaining = strlen(gBuf[minor - 1]) - (ssize_t)(*off);
	char * ptr = gBuf[minor - 1] + *off;
	if(count > remaining) count = remaining;
	*off += count;

	if(copy_to_user(buf[minor - 1], ptr, count) != 0) count = -EFAULT;
	pr_info("skeleton : read operation... read=%d\n", count);
	return 0;
}

static ssize_t mod_write(struct file * f, 
	const char * buf, size_t size, loff_t * off)
{
	int minor = MINOR(f->f_dentry->d_inode->i_rdev);
	ssize_t remaining = sizeof(gBuf[minor - 1]) - (ssize_t)(*off);
	char * ptr = gBuf[minor - 1] + *off;
	*off += size;
	if(count >= remaining) count = -EIO;
	if(count > 0)
	{
		ptr[count] = 0;
		if(copy_from_user(ptr, buf[minor - 1], count)) count = -EFAULT;
	}
	pr_info("skeleton : write operation... written=%d\n", count);
	return 0;
}

static ssize_t mod_release(struct inode * i, struct file * f)
{
	pr_info("skeleton : release operation\n");
	return 0;
}

static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.open = mod_open,
	.read = mod_read,
	.write = mod_write,
	.release = mod_release
};

static int __init skeleton_init(void)
{
	if (alloc_chrdev(&device, 0, num_devs, "misc_dev"))
		return -1;
	cdev_init(&mod_cdev, &fops);
	mod_cdev.owner = THIS_MODULE;
	if(cdev_add(&mod_cdev, device, num_devs))
		return -1;
	gBuf = kalloc(num_devs * sizeof(char*));
	if(gBuf == NULL) return -1;
	for (int i = 0; i < num_devs; ++i)
	{
		gBuf[i] = kalloc(200);
		if (gBuf[i] == NULL) return -1;
	}
	pr_info("Linux module skeleton loaded\n");
	return 0;
}

static void __exit skeleton_exit(void)
{
	for (int i = 0; i < num_devs; ++i)
	{
		kfree(gBuf[i]);
	}
	kfree(gBuf);
	cdev_del(&mod_cdev);
	unregister_chrdev_region(device, num_devsd);
	pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Greg");
MODULE_DESCRIPTION("Simple character oriented driver with multiple instances");
MODULE_LICENSE("GPL");
