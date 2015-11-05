#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include "linux/fs.h"
#include "linux/kdev_t.h"
#include "linux/cdev.h"
#include "linux/uaccess.h"

static int major = 0; /* Major number */

#define BUF_SIZE 64
static char buffer[BUF_SIZE];
static size_t num = 0; /* Number of available bytes in the buffer */

static ssize_t mod_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	size_t real;
	real = min((size_t)BUF_SIZE, count);
	if (real)
		if (copy_from_user(buffer, buf, real))
			return -EFAULT;
	num = real; /* Destructive write (overwrite previous data if any) */
	pr_info("mod: write %s\n", buffer);
	return real;
}

static ssize_t mod_read(struct file *file, char *buf, size_t count,
loff_t *ppos)
{
	size_t real;
	real = min(num, count);
	if (real)
		if (copy_to_user(buf, buffer, real))
			return -EFAULT;
	num = 0; /* Destructive read (no more data after a read) */
	pr_info("mod: read %s\n", buffer);
	return real;
}

static int mod_open(struct inode *inode, struct file *file)
{
	pr_info("mod: open()\n");
	return 0;
}

static int mod_release(struct inode *inode, struct file *file)
{
	pr_info("mod: release()\n");
	return 0;
}

static struct file_operations mod_fops = {
	.owner = THIS_MODULE,
	.read = mod_read,
	.write = mod_write,
	.open = mod_open,
	.release = mod_release,
};

static int __init mod_init(void)
{
	int ret;
	ret = register_chrdev(major, "mod", &mod_fops);
	if (ret < 0) {
		pr_info("mod: unable to get a major\n");
		return ret;
	}
	if (major == 0)
		major = ret; /* dynamic value */
	pr_info("mod: successfully loaded with major %d\n",major);
	return 0;
}

static void __exit mod_exit(void)
{
	unregister_chrdev(major, "mod");
	pr_info("mod: successfully unloaded\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_AUTHOR("Emilie Gsponer");
MODULE_DESCRIPTION("drive oriented character");
MODULE_LICENSE("GPL");

