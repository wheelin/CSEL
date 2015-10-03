/*skeleton.c*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h> 

#define CHECK_RET(ret) if(ret != 0) return -1;

/*First argument, 50 length's string*/
static char* text="dummy help";
module_param(text, charp, 0);

/*Second argument, int */
static int element_num = 1;
module_param(element_num, int, 0);

static int 		Pin_nr 			= 4;
static int 		SW_pin_nr[4] 	= {5, 6, 6, 2};
static int 		SW_io_nr[4] 	= {29, 30, 22, 18};
static int 		SW_ID[4]		= {100, 101, 102, 103};
static char * 	SW_name[4] 		= {"SW1","SW2", "SW3","SW4"};

irqreturn_t switch_irq_handler(int irq, void *dev_id)
{
	pr_info("%s has been pressed!!!", SW_name[(*dev_id) - 100]);
	return IRQ_HANDLED;
}

static int __init skeleton_init(void)
{
	int ret;
	int i;
	
	ret = -1;
	pr_info("Interrupt handler module loaded in kernel");

	pr_info("Configuring pins");
	for(i = 0; i < Pin_nr; i++)
	{
		if(i < 2)
		{
			ret = gpio_request(EXYNOS5_GPX2(SW_pin_nr[i]));
			ret = gpio_direction_imput(EXYNOS5_GPX2(SW_pin_nr[i]));
		} else
		{
			ret = gpio_request(EXYNOS5_GPX1(SW_pin_nr[i]));
			ret = gpio_direction_imput(EXYNOS5_GPX1(SW_pin_nr[i]));
			
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
	return ret;
}

static void __exit skeleton_exit(void)
{
	int i;
	pr_info("Freeing interrupts");
	for(i = 0; i < Pin_nr; i++)
		free_irq(gpio_to_irq(SW_io_nr[i]), SW_ID[i]);
	pr_info("Interrupts freed");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Greg");
MODULE_DESCRIPTION("MISC");
MODULE_LICENSE("GPL");
