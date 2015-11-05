#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

static char* text = "hello";
module_param(text,charp,0);

static int elements=2;
module_param(elements,int,0);

static int __init skeleton_init(void)
{
	pr_info("Linux module skeleton loaded\n");
	pr_info("text:%s\nelements:%d\n",text,elements);
	return 0;
}

static void __exit skeleton_exit(void)
{
	pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Emilie Gsponer");
MODULE_DESCRIPTION("Module with parameters");
MODULE_LICENSE("GPL");
