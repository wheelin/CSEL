#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int __init skeleton_init(void)
{
	pr_info("Linux module skeleton loaded\n");
	return 0;
}

static void __exit skeleton_exit(void)
{
	pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Emilie Gsponer");
MODULE_DESCRIPTION("Module Skeleton");
MODULE_LICENSE("GPL");
