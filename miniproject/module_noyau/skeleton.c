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

static struct pwm_device * pwm_dev;

static Mode_t gMode;
static int gDuty;
static char auto_str[] = "auto\n";
static char man_str[] = "manual\n";
static char cmd[CMD_MAX_LEN];
struct task_struct *regulation_task;
static int thread_is_running;
/******************************************************************************
**
******************************************************************************/
static void pwm_init(void)
{
	pwm_dev = pwm_request(0, "myPWM");
}

static void pwm_en(void)
{
	pwm_enable(pwm_dev);
}

static void pwm_dis(void)
{
	pwm_disable(pwm_dev);
}

static void pwm_release(void)
{
	pwm_put(pwm_dev);
}
/******************************************************************************
**
******************************************************************************/
static void set_pwm(unsigned int percent)
{
	unsigned long duty;
	if(percent > 100) 
	{
		duty = PERIOD;
		gDuty = 100;
	}
	else if(percent < 0) 
	{
		duty = 0;
		gDuty = 0;
	}
	else
	{
		duty = (PERIOD * percent)/100;	
		gDuty = percent;
	}
	pr_info("Duty set to : %ld\n", duty);
	if(duty != gDuty)
	{
		pwm_config(pwm_dev, duty, PERIOD);
	}
}

/******************************************************************************
**
******************************************************************************/
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
			{
				set_pwm(0);
				pwm_dis();
			}
			else if(temp >= 57 && temp < 63)
			{
				set_pwm(20);
				pwm_en();
			}
			else if(temp >= 63 && temp < 68)
			{
				set_pwm(50);
				pwm_en();
			}
			else if(temp >= 68)
			{
				set_pwm(100);
				pwm_en();
			}
			pr_info("Temperature of exynos is : %ld.\n", temp);
		}
		msleep(2000);
	}
    return 0;
}

/******************************************************************************
**
******************************************************************************/
static ssize_t skeleton_show_mode(struct device *dev, 
			struct device_attribute *attr,
			char *buf)
{
	if(gMode == AUTO)
	{
		memcpy(buf, auto_str, sizeof(auto_str));
	}
	else
	{
		memcpy(buf, man_str, sizeof(man_str));
	}
	return sizeof(buf);
}

/******************************************************************************
**
******************************************************************************/
static ssize_t skeleton_set_mode(struct device *dev, 
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	if(count <= CMD_MAX_LEN)
	{
		memcpy(cmd, buf, count);
	}
	else
	{
		return 0;
	}
	if(strncmp(cmd, auto_str, count) == 0)
	{
		gMode = AUTO;
		pr_info("Fan Control : chanded mode to automatic\n");
		if(thread_is_running == 0)
			regulation_task = kthread_run(&thread_regulate_fan, 
										NULL, "regulation_task");
		thread_is_running = 1;
	}
	else if(strncmp(cmd, man_str, count) == 0)
	{
		gMode = MAN;
		pr_info("Fan Control : changed mode to manual\n");
		set_pwm(50);
		pwm_en();
		if(thread_is_running == 1)
			kthread_stop(regulation_task);
		thread_is_running = 0;
	}
	else gMode = AUTO;
	return count;
}

/******************************************************************************
**
******************************************************************************/
static ssize_t skeleton_show_duty(struct device *dev, 
			struct device_attribute *attr,
			char *buf)
{
	char duty_str[15];
	sprintf(duty_str, "%d", gDuty);
	memcpy(buf, duty_str, sizeof(duty_str));
	return sizeof(buf);
}

/******************************************************************************
**
******************************************************************************/
static ssize_t skeleton_set_duty(struct device *dev, 
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	char * ret;
	int duty;
	duty = simple_strtoul(buf, &ret, 10);
	pr_info("Fan Control : duty of %d trying to be set\n", duty);	
	if(gMode == MAN)
	{
		pr_info("Fan Control : duty of %d was set\n", duty);
		set_pwm(duty);
		pwm_en();
	}
	return sizeof(buf);
}

/******************************************************************************
**
******************************************************************************/
static void sysfs_dev_release(struct device *dev) 
{ 
	kthread_stop(regulation_task);
	pwm_dis();
	pwm_release();
} 

/******************************************************************************
**
******************************************************************************/
DEVICE_ATTR(mode, 0666, skeleton_show_mode, skeleton_set_mode);
DEVICE_ATTR(duty, 0666, skeleton_show_duty, skeleton_set_duty);


static struct platform_driver sysfs_driver = {
	.driver = {.name = "Fan_control_module"},
};

static struct platform_device sysfs_device = {
	.name = "Fan_control_module",
	.id = -1,
	.dev.release = sysfs_dev_release,
};

/******************************************************************************
**
******************************************************************************/
static int __init skeleton_init(void)
{
	
	int status;
	status = platform_driver_register(&sysfs_driver);
	if(status != 0) 
	{
		return status;
	}
	status = platform_device_register(&sysfs_device);
	if(status != 0) 
	{
		return status;
	}
	status = device_create_file(&sysfs_device.dev, &dev_attr_mode);
	if(status != 0) 
	{	
		pr_info("Problem creating device attribute file");
		return status;
	}
	status = device_create_file(&sysfs_device.dev, &dev_attr_duty);
	if(status != 0) 
	{
		pr_info("Problem creating device attribute file");
		return status;
	}
	regulation_task = kthread_run(&thread_regulate_fan, 
										NULL, "regulation_task");
	thread_is_running = 1;
	pwm_init();
	gMode = AUTO;
	pr_info("Linux module skeleton loaded\n");
	return 0;
}

/******************************************************************************
**
******************************************************************************/
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
MODULE_DESCRIPTION("Module to control the fan");
MODULE_LICENSE("GPL");
