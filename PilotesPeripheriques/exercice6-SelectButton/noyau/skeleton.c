/*skeleton.c*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h> 

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/wait.h>

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>

#define CHECK_RET(ret) if(ret != 0) return -1;

static int 		Pin_nr 			= 4;
static int 		GPIO_nr			= {2,2,1,1};
static int 		SW_pin_nr[4] 	= {5, 6, 6, 2};
static int 		SW_io_nr[4] 	= {29, 30, 22, 18};
static int 		SW_ID[4]		= {100, 101, 102, 103};
static char * 		SW_name[4] 		= {"SW1","SW2", "SW3","SW4"};

int request_can_be_processed = 0;
int data1 = 20;
struct task_struct *task;

static int major = 0; /* Major number */

//Wait queue declaration
wait_queue_head_t wait_queue;

// Thread wait for event
int thread(void *data)
{
	while(!kthread_should_stop())
	{
		wait_event_interruptible(wait_queue,request_can_be_processed !=0);
		pr_info("Thread awake\n");
	}
    	return 0;
}

irqreturn_t switch_irq_handler(int irq, void *dev_id)
{
	pr_info("Some switch has been pressed\n");
	request_can_be_processed = 1;
	wake_up_interruptible(&wait_queue);
	return IRQ_HANDLED;
}

static int mod_open(struct inode *inode, struct file *file)
{
	pr_info("mod: open\n");

	// Create and start the threads
	task = kthread_run(&thread,(void *)data1,"Thread");
	pr_info("Threads started\n");

	return 0;
}

static int mod_release(struct inode *inode, struct file *file)
{
	pr_info("mod: release\n");

	kthread_stop(task);
	pr_info("Threads stopped\n");	

	return 0;
}

static unsigned int mod_poll(struct file *f, poll_table *wait)
{
	unsigned mask = 0;
	poll_wait(f,&wait_queue,wait);
	if(request_can_be_processed!=0)
		mask |= POLLIN|POLLRDNORM;
	request_can_be_processed  = 0;
	return mask;
}

static struct file_operations mod_fops = {
	.owner = THIS_MODULE,
	.open = mod_open,
	.release = mod_release,
	.poll = mod_poll,
};

static int __init skeleton_init(void)
{
	int ret;
	int i;

	int ret1;
	ret1 = register_chrdev(major, "mod", &mod_fops);
	if (ret1 < 0) {
		pr_info("mod: unable to get a major\n");
		return ret;
	}
	if (major == 0)
		major = ret1; /* dynamic value */
	pr_info("mod: successfully loaded with major %d\n",major);
	
	ret = -1;
	pr_info("Interrupt handler module loaded in kernel");

	pr_info("Configuring pins");
	for(i = 0; i < Pin_nr; i++)

	{
		if(i < 2)
		{
			ret = gpio_request(EXYNOS5_GPX2(SW_pin_nr[i]), SW_name[i]);
			ret = gpio_direction_input(EXYNOS5_GPX2(SW_pin_nr[i]));
		} else
		{
			ret = gpio_request(EXYNOS5_GPX1(SW_pin_nr[i]), SW_name[i]);
			ret = gpio_direction_input(EXYNOS5_GPX1(SW_pin_nr[i]));	
		}
		CHECK_RET(ret);
	}

	pr_info("Configuring switches interrupts");
	for(i = 0; i < Pin_nr; i++)
	{
		ret = request_irq(gpio_to_irq(SW_io_nr[i]),
			switch_irq_handler,
			IRQF_SHARED|IRQF_TRIGGER_RISING,
			SW_name[i],
			&SW_ID[i]);
		CHECK_RET(ret);
	}
	
	pr_info("Pins and interrupts have been configured.");

	pr_info("Init wait queue\n");
	// Initialize wait queue
	init_waitqueue_head(&wait_queue);
	
	return ret;
}

static void __exit skeleton_exit(void)
{
	int i;
	pr_info("Freeing interrupts");
	for(i = 0; i < Pin_nr; i++)
		free_irq(gpio_to_irq(SW_io_nr[i]), &SW_ID[i]);
	pr_info("Interrupts freed");

	unregister_chrdev(major, "mod");
	pr_info("mod: successfully unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Emilie Gsponer");
MODULE_DESCRIPTION("Operation bloquante");
MODULE_LICENSE("GPL");
