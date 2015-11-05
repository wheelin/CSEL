#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

int data = 20;
struct task_struct *task;

// Thread function body
int thread(void *data)
{
	while(!kthread_should_stop())
	{
		ssleep(5);
		pr_info("Thread awake\n");
	}
    	return 0;
}

static int __init skeleton_init(void)
{
	pr_info("Thread created\n");
	// Create and start the thread
	task = kthread_run(&thread,(void *)data,"Thread_1");
	return 0;
}

static void __exit skeleton_exit(void)
{
	// Stop the thread
	kthread_stop(task);
	pr_info("Thread stopped\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Emilie Gsponer");
MODULE_DESCRIPTION("Thread sleep");
MODULE_LICENSE("GPL");
