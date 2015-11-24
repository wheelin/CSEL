
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <syslog.h>


/*
 * pwm0 - gpb2.0 - 203
 */
#define GPIO_EXPORT	"/sys/class/gpio/export"
#define GPIO_UNEXPORT	"/sys/class/gpio/unexport"
#define GPIO_PWM	"/sys/class/gpio/gpio203"
#define PWM		"203"
#define GPIO_SW1	"/sys/class/gpio/gpio29"
#define SW1		"29"
#define GPIO_SW2	"/sys/class/gpio/gpio30"
#define SW2		"30"
#define GPIO_SW3	"/sys/class/gpio/gpio22"
#define SW3		"22"
#define GPIO_LED1	"/sys/class/gpio/gpio31"
#define LED1		"31"
#define GPIO_LED2	"/sys/class/gpio/gpio28"
#define LED2		"28"
#define GPIO_LED3	"/sys/class/gpio/gpio19"
#define LED3		"19"

#define CLOCK_TIME_NS 50000
static int timerfd1;
static int epollfd;
static int duty_cycle;

static int timeDuty1;
static int timeDuty2;
static bool state;
static bool sw1_state;
static bool sw3_state;

#define MAX_EVENTS 4
struct epoll_event evtm1,evsw1,evsw2,evsw3, events[MAX_EVENTS];
static struct itimerspec timer_value1,timer_value2;

static void create_epoll()
{
	// Create epoll
	epollfd = epoll_create1(0); 
	if (epollfd == -1)
	{
		perror ("epoll_create");
		exit (1);
	}
}
static void create_timer()
{
	/* Create the timer */
	timerfd1 = timerfd_create (CLOCK_MONOTONIC, 0);
	if (timerfd1 == -1)
	{
		perror ("timerfd1_create");
		exit (1);
	}

	// Assign the timer to epoll
	evtm1.events = EPOLLIN | EPOLLOUT;
	evtm1.data.fd = timerfd1;

	int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd1, &evtm1);
	if(ret == -1)
	{
		perror ("epoll_ctl1");
		exit (1);
	}

	if(timeDuty1 != 0)
	{
		state = 0;
		if (timerfd_settime(timerfd1, 0, &timer_value1, NULL) == -1)
		{
			perror ("timerfd1_settime");
			exit (1);
		}
	}
	else if(timeDuty2 != 0)
	{
		state = 0;
		if (timerfd_settime(timerfd1, 0, &timer_value2, NULL) == -1)
		{
			perror ("timerfd2_settime");
			exit (1);
		}
	}

	setlogmask(LOG_UPTO(LOG_NOTICE));
	openlog("fan control",LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
}
static int open_pwm()
{
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	write (f, PWM, strlen(PWM));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	write (f, PWM, strlen(PWM));
	close (f);	

	// config pin
	f = open (GPIO_PWM "/direction", O_WRONLY);
	write (f, "out", strlen("out"));
	close (f);

	// open gpio value attribute
 	f = open (GPIO_PWM "/value", O_RDWR);
	return f;
}
static int open_sw1()
{
	int ret;
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	write (f, SW1, strlen(SW1));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	write (f, SW1, strlen(SW1));
	close (f);	

	// config pin
	f = open (GPIO_SW1 "/direction", O_WRONLY);
	write (f, "in", strlen("in"));
	close (f);

	// set edge
	f = open (GPIO_SW1 "/edge", O_WRONLY);
	write (f, "both", strlen("both"));
	close (f);

	// open gpio value attribute
 	f = open (GPIO_SW1 "/value", O_RDWR);

	// Assign the switch to epoll
	evsw1.events = EPOLLPRI;
	evsw1.data.fd = f;

	ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, f, &evsw1);
	if(ret == -1)
	{
		perror ("epoll add sw1");
		exit (1);
	}
	sw1_state = false;
	return f;
}
static int open_sw2()
{
	int ret;
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	write (f, SW2, strlen(SW2));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	write (f, SW2, strlen(SW2));
	close (f);	

	// config pin
	f = open (GPIO_SW2 "/direction", O_WRONLY);
	write (f, "in", strlen("in"));
	close (f);

	// set edge
	f = open (GPIO_SW2 "/edge", O_WRONLY);
	write (f, "falling", strlen("falling"));
	close (f);

	// open gpio value attribute
 	f = open (GPIO_SW2 "/value", O_RDWR);

	// Assign the switch to epoll
	evsw2.events = EPOLLPRI;
	evsw2.data.fd = f;

	ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, f, &evsw2);
	if(ret == -1)
	{
		perror ("epoll add sw2");
		exit (1);
	}
	return f;
}
static int open_sw3()
{
	int ret;
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	write (f, SW3, strlen(SW3));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	write (f, SW3, strlen(SW3));
	close (f);	

	// config pin
	f = open (GPIO_SW3 "/direction", O_WRONLY);
	write (f, "in", strlen("in"));
	close (f);

	// set edge
	f = open (GPIO_SW3 "/edge", O_WRONLY);
	write (f, "both", strlen("both"));
	close (f);

	// open gpio value attribute
 	f = open (GPIO_SW3 "/value", O_RDWR);

	// Assign the switch to epoll
	evsw3.events = EPOLLPRI;
	evsw3.data.fd = f;

	ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, f, &evsw3);
	if(ret == -1)
	{
		perror ("epoll add sw3");
		exit (1);
	}
	sw3_state = false;
	return f;
}

static int open_led1()
{
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	write (f, LED1, strlen(LED1));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	write (f, LED1, strlen(LED1));
	close (f);	

	// config pin
	f = open (GPIO_LED1 "/direction", O_WRONLY);
	write (f, "out", strlen("out"));
	close (f);

	// open gpio value attribute
 	f = open (GPIO_LED1 "/value", O_RDWR);
	return f;
}

static int open_led2()
{
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	write (f, LED2, strlen(LED2));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	write (f, LED2, strlen(LED2));
	close (f);	

	// config pin
	f = open (GPIO_LED2 "/direction", O_WRONLY);
	write (f, "out", strlen("out"));
	close (f);

	// open gpio value attribute
 	f = open (GPIO_LED2 "/value", O_RDWR);
	return f;
}

static int open_led3()
{
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	write (f, LED3, strlen(LED3));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	write (f, LED3, strlen(LED3));
	close (f);	

	// config pin
	f = open (GPIO_LED3 "/direction", O_WRONLY);
	write (f, "out", strlen("out"));
	close (f);

	// open gpio value attribute
 	f = open (GPIO_LED3 "/value", O_RDWR);
	return f;
}

static void setDutyCycle(int value_percent)
{	
	int ret;
	duty_cycle = value_percent;
	timeDuty1 = CLOCK_TIME_NS*value_percent/100;
	timeDuty2 = CLOCK_TIME_NS*(100-value_percent)/100;

 	syslog(LOG_NOTICE, "duty :%d",value_percent);

	/* Make the timer periodic */
	timer_value1.it_interval.tv_sec = 0;
	timer_value1.it_interval.tv_nsec = timeDuty1;
	timer_value1.it_value.tv_sec = 0;
	timer_value1.it_value.tv_nsec = timeDuty1;

	timer_value2.it_interval.tv_sec = 0;
	timer_value2.it_interval.tv_nsec = timeDuty2;
	timer_value2.it_value.tv_sec = 0;
	timer_value2.it_value.tv_nsec = timeDuty2;
}

int main(int argc, char* argv[]) 
{
	// Initialize variable
	int nr;
	int fSw1,fSw2,fSw3;
	int led1,led2,led3;
	uint64_t timersElapsed = 0;
	int i=0;
	int time_counter = 0;

	// Configure PWM
 	int pwm = open_pwm();
	pwrite (pwm, "1", sizeof("1"), 0);

	// Configure LEDs
	led1 = open_led1();
	pwrite (led1, "0", sizeof("0"), 0);

	led2 = open_led2();
	pwrite (led2, "0", sizeof("0"), 0);

	led3 = open_led3();
	pwrite (led3, "0", sizeof("0"), 0);
	
	// Set duty cycle
	if (argc >= 2)   
		setDutyCycle(atoi (argv[1]));
	else
		setDutyCycle(50);

	// Create epoll and timer
	create_epoll();
	create_timer();

	// Configure Switch
	fSw1 = open_sw1();
	fSw2 = open_sw2();
	fSw3 = open_sw3();
	read (fSw1, &timersElapsed,8);
	read (fSw2, &timersElapsed,8);
	read (fSw3, &timersElapsed,8);

	while(1)
	{
		// Wait for timer event
		nr = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if(nr == -1)
		{
			perror ("epoll faillure");
			exit (1);
		}
		for(i=0;i<nr;i++)
		{
			// Sw1 event : Up duty cycle
			if(events[i].data.fd == fSw1)
			{
				read (fSw1, &timersElapsed,8);
				sw1_state = !sw1_state;
				if(sw1_state)
				{
					time_counter = 0;
					if(duty_cycle < 100)
						setDutyCycle(duty_cycle+2);
					pwrite (led1, "1", sizeof("1"), 0);
				}
				else
					pwrite (led1, "0", sizeof("0"), 0);
			}
			// Sw2 event : 50% duty cycle
			else if(events[i].data.fd == fSw2)
			{
				read (fSw2, &timersElapsed,8);
				setDutyCycle(50);
			}
			// Sw3 event : Down duty cycle
			else if(events[i].data.fd == fSw3)
			{
				read (fSw3, &timersElapsed,8);
				sw3_state = !sw3_state;
				if(sw3_state)
				{
					time_counter = 0;
					if(duty_cycle > 0)
						setDutyCycle(duty_cycle-2);	
					pwrite (led3, "1", sizeof("1"), 0);
				}
				else
					pwrite (led3, "0", sizeof("0"), 0);
			}
			// Timer event : Generate fan pwm
			else if(events[i].data.fd == timerfd1)
			{
				read (timerfd1, &timersElapsed,8);
				// 0% duty
				if(duty_cycle == 0)
				{
					pwrite (pwm, "0", sizeof("0"), 0);
					pwrite (led2, "0", sizeof("0"), 0);
				}
				// 100% duty
				else if(duty_cycle == 100)
				{
					pwrite (pwm, "1", sizeof("1"), 0);
					pwrite (led2, "1", sizeof("1"), 0);
				}
				// other duty
				else
				{
					// PWM high
					if(state)
					{
						if (timerfd_settime(timerfd1, 0, &timer_value1, NULL)==-1)
						{
							perror ("timerfd2_settime");
							exit (1);
						}
						state = !state;
						pwrite (pwm, "1", sizeof("1"), 0);
						pwrite (led2, "1", sizeof("1"), 0);
					}
					// PWM low
					else
					{
						if (timerfd_settime(timerfd1, 0, &timer_value2, NULL)==-1)
						{
							perror ("timerfd1_settime");
							exit (1);
						}
						state = !state;
						pwrite (pwm, "0", sizeof("0"), 0);
						pwrite (led2, "0", sizeof("0"), 0);

						// Manage long press on sw1 and/or sw3
						if((sw3_state != sw1_state) && (sw3_state| sw1_state))
							time_counter++;
						if(time_counter >= 2000)
						{
							time_counter = 0;
							if(sw3_state && duty_cycle > 0)
								setDutyCycle(duty_cycle-2);
							if(sw1_state && duty_cycle < 100)
								setDutyCycle(duty_cycle+2);
						}
					}
				}
			}
		}
	}
	
	return 0;
}

