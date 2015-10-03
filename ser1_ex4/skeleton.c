/*skeleton.c*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/list.h> 

typedef struct
{
	int id;
	char str[50];
	struct list_head node;
} Element;

static LIST_HEAD(list);

/*First argument, 50 length's string*/
static char* text="dummy help";
module_param(text, charp, 0);

/*Second argument, int */
static int element_num = 1;
module_param(element_num, int, 0);

static int __init skeleton_init(void)
{
	int i;
	int id;
	Element * ele;
	
	id = 0;
	for(i = 0; i < element_num; i++)
	{		
		ele = kzalloc(sizeof(Element), GFP_KERNEL);
		if(ele == NULL)	/*Didn't allocate memory for the struct*/
			return -1;
		memcpy(ele->str, text, strlen(text));
		ele->id = id++;
		list_add_tail(&ele->node, &list);
		pr_info("New element added to the list\n");
	}
	
	/*List code handling*/
	list_for_each_entry(ele, &list, node)
	{
		pr_info("ID of element : %d\nString is : %s\n", ele->id, ele->str);
	}
	
	return 0;
}

static void __exit skeleton_exit(void)
{
	Element * ele;
	list_for_each_entry(ele, &list, node)
	{
		kfree(ele);
		pr_info("Element poped\n");
	}
	list_del(&ele->node);
	pr_info("Module removed");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Greg");
MODULE_DESCRIPTION("MISC");
MODULE_LICENSE("GPL");
