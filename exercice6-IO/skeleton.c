#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/ioport.h>

static int __init skeleton_init(void)
{
	pr_info("Linux module skeleton loaded\n");
	
	// Allocate memory region
	request_mem_region(0x10000000,0x100,"uP register");

	pr_info("Memory allocated\n");

	return 0;
}

static void __exit skeleton_exit(void)
{
	pr_info("Linux module skeleton unloaded\n");

	// Free memory region
	release_mem_region(0x10000000,0x100);

	pr_info("Memory released\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Emilie Gsponer");
MODULE_DESCRIPTION("Module Skeleton");
MODULE_LICENSE("GPL");
