#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include "linux/fs.h"
#include "linux/kdev_t.h"
#include "linux/kthread.h"
#include "linux/delay.h"
#include "linux/pwm.h"
#include "linux/device.h"
#include "linux/platform_device.h"
#include "linux/string.h"

#define PERIOD 20972
#define CMD_MAX_LEN 50
typedef enum {AUTO, MAN} Mode_t;

extern unsigned long exynos_thermal_get_value(void);

static struct pwm_device pwm_struct;

static Mode_t gMode;
static int gDuty;
static char * auto_str = "auto";
static char * man_str = "manual";

static void set_pwm(unsigned int percent)
{
	unsigned long duty;
	if(percent > 100) duty = PERIOD;
	else if(percent < 0) duty = 0;
	duty = PERIOD * (double)(percent/100);	
	pwm_config(&pwm_struct, duty, PERIOD);
	gDuty = duty;
	pr_info("Duty cycle changed\n");
}

struct task_struct *regulation_task;
int thread_regulate_fan(void *data)
{
	unsigned long temp;
	pr_info("Thread awake\n");
	while(!kthread_should_stop())
	{
		if(gMode == AUTO)
		{
			temp = exynos_thermal_get_value();
			if(temp < 57)
				pwm_disable(&pwm_struct);
			else if(temp >= 57 && temp < 63)
			{
				pwm_enable(&pwm_struct);
				set_pwm(20);
			}
			else if(temp >= 63 && temp < 68)
			{
				pwm_enable(&pwm_struct);
				set_pwm(50);
			}
			else if(temp >= 68)
			{
				pwm_enable(&pwm_struct);
				set_pwm(100);
			}
		}
		msleep(100);
	}
    	return 0;
}

static ssize_t skeleton_show_mode(struct device_driver * dev, 
	char * buf)
{
	if(gMode == AUTO)
		buf = auto_str;
	else
		buf = man_str;
	return sizeof(buf);
}

static ssize_t skeleton_set_mode(struct device_driver * dev,
	const char * buf, size_t count)
{
	if(strncmp(buf, auto_str, 3) == 0)
	{
		gMode = AUTO;
		regulation_task = kthread_run(&thread_regulate_fan, 
										NULL, "regulation_task");
	}
	else if(strncmp(buf, man_str, 3) == 0)
	{
		gMode = MAN;
		kthread_stop(regulation_task);
	}
	else gMode = AUTO;
	return 0;
}

static ssize_t skeleton_show_duty(struct device_driver * dev, 
	char * buf)
{
	char duty_str[15];
	sprintf(duty_str, "%d", gDuty);
	buf = duty_str;
	return sizeof(buf);
}

static ssize_t skeleton_set_duty(struct device_driver * dev,
	const char * buf, size_t count)
{
	char * ret;
	if(gMode == MAN)
		set_pwm((int)simple_strtoul(buf, &ret, 10));
	return sizeof(buf);
}

DEVICE_ATTR(mode, 0666, skeleton_show_mode, skeleton_set_mode);
DEVICE_ATTR(duty, 0666, skeleton_show_duty, skeleton_set_duty);

static void sysfs_dev_release(struct device *dev) 
{ 
	pwm_put(&pwm_struct);
} 

static struct platform_driver sysfs_driver = {
	.driver = {.name = "Fan_control_module"},
};

static struct platform_device sysfs_device = {
	.name = "Fan_control_module",
	.id = -1,
	.dev.release = sysfs_dev_release,
};

static int __init skeleton_init(void)
{
	
	int status;
	status = platform_driver_register(&sysfs_driver);
	if(status != 0) 
	{
		return status;
	}
	status = platform_device_register(&sysfs_device);
	if(status != 0) return status;
	device_create_file(&sysfs_device.dev, &dev_attr_mode);
	device_create_file(&sysfs_device.dev, &dev_attr_duty);
	//start of the fan control initialization section
	pwm_struct = *pwm_request(0, "myPWM");
	if(pwm_enable(&pwm_struct)) return -1;

	pr_info("Linux module skeleton loaded\n");
	return 0;
}

static void __exit skeleton_exit(void)
{
	device_remove_file(&sysfs_device.dev, &dev_attr_mode);
	device_remove_file(&sysfs_device.dev, &dev_attr_duty);
	platform_device_unregister(&sysfs_device);
	platform_driver_unregister(&sysfs_driver);
	pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Greg");
MODULE_DESCRIPTION("Simple character oriented driver");
MODULE_LICENSE("GPL");
