#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/wait.h>

int data1 = 20;
int data2 = 30;
struct task_struct *task1;
struct task_struct *task2;
static int flag = 0;

//Wait queue declaration
wait_queue_head_t wait_queue;

// Thread wait for event
int thread1(void *data)
{
	while(!kthread_should_stop())
	{
		wait_event_interruptible(wait_queue,flag!=0);
		pr_info("Thread1 (wait notif) awake\n");
		flag = 0;
	}
    	return 0;
}

// Thread send notification each 5s
int thread2(void *data)
{
	while(!kthread_should_stop())
	{
		ssleep(5);
		pr_info("Thread2 (notif each 5s) awake\n");
		flag = 1;
		wake_up(&wait_queue);
	}
    	return 0;
}

static int __init skeleton_init(void)
{
	pr_info("Init wait queue\n");
	// Initialize wait queue
	init_waitqueue_head(&wait_queue);
	
	pr_info("Threads created\n");
	// Create and start the threads
	task1 = kthread_run(&thread1,(void *)data1,"Thread_1");
	task2 = kthread_run(&thread2,(void *)data2,"Thread_2");
	return 0;
}

static void __exit skeleton_exit(void)
{
	// Stop the threads
	kthread_stop(task1);
	kthread_stop(task2);
	pr_info("Threads stopped\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Emilie Gsponer");
MODULE_DESCRIPTION("Module Skeleton");
MODULE_LICENSE("GPL");
