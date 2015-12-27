/**
 * Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Project:	HEIA-FR / HES-SO MSE - MA-CSEL1 Laboratory
 *
 * Abstract: 	System programming -  file system
 *
 * Purpose:	ODROID-XU3 Lite silly fan control system
 *
 * Autĥor:	Daniel Gachet
 * Date:	05.11.2015
 */
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
#include <signal.h>
#include <syslog.h>
#include <pwd.h>

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

#define UNUSED(x) (void)(x)

static int signal_catched = 0;

static int epollfd;
static int timerfd1;

static bool sw1_state;
static bool sw2_state;
static bool sw3_state;

int isAuto;
int dutyCycle;

static int led3;

static char * auto_str = "auto";
static char * man_str = "manual";
static char * duty_str = "duty";

int fdNoyauDuty, fdNoyauMode;

#define MAX_EVENTS 3
struct epoll_event evsw1,evtm1, evsw2,evsw3, events[MAX_EVENTS];
static struct itimerspec timer_value1;

static void create_epoll()
{
	// Create epoll
	epollfd = epoll_create1(0); 
	if (epollfd == -1)
	{
		syslog (LOG_ERR,"epoll_create");
		exit (1);
	}
}

static void create_timer()
{
	/* Create the timer */
	timerfd1 = timerfd_create (CLOCK_MONOTONIC, 0);
	if (timerfd1 == -1)
	{
		syslog (LOG_ERR,"timerfd1_create");
		exit (1);
	}

	// Assign the timer to epoll
	evtm1.events = EPOLLIN | EPOLLOUT;
	evtm1.data.fd = timerfd1;

	int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd1, &evtm1);
	if(ret == -1)
	{
		syslog (LOG_ERR,"epoll_ctl1");
		exit (1);
	}

	/* Make the timer periodic */
	timer_value1.it_interval.tv_sec = 0;
	timer_value1.it_interval.tv_nsec = CLOCK_TIME_NS;
	timer_value1.it_value.tv_sec = 0;
	timer_value1.it_value.tv_nsec = CLOCK_TIME_NS;

	if (timerfd_settime(timerfd1, 0, &timer_value1, NULL) == -1)
	{
		syslog (LOG_ERR,"timerfd2_settime");
		exit (1);
	}
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
		syslog (LOG_ERR,"epoll add sw1");
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
	write (f, "both", strlen("both"));
	close (f);

	// open gpio value attribute
 	f = open (GPIO_SW2 "/value", O_RDWR);

	// Assign the switch to epoll
	evsw2.events = EPOLLPRI;
	evsw2.data.fd = f;

	ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, f, &evsw2);
	if(ret == -1)
	{
		syslog (LOG_ERR,"epoll add sw2");
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
		syslog (LOG_ERR,"epoll add sw3");
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

static void open_noyau_dutycomm()
{
	fdNoyauDuty = open ("/sys/devices/platform/Fan_control_module/duty", O_RDWR);
	if (fdNoyauDuty < 0) {
		syslog (LOG_ERR,"open noyau duty");
	}
}

static void open_noyau_modecomm()
{
	fdNoyauMode = open ("/sys/devices/platform/Fan_control_module/mode", O_RDWR);
	if (fdNoyauMode < 0) {
		syslog (LOG_ERR,"open noyau mode");
	}
}

static void setDutyCycle(int value_percent)
{	
	dutyCycle = value_percent;
	syslog (LOG_INFO, "daemon Duty %d", dutyCycle);
	char buffer[10];
	sprintf(buffer,"%d",dutyCycle);
	if(fdNoyauDuty<0)
		open_noyau_dutycomm();
	if(fdNoyauDuty>= 0)
		write (fdNoyauDuty, buffer, sizeof(buffer));
}

static void setMode(bool mode)
{	
	if(isAuto != mode)
	{
		isAuto = mode;
		if(fdNoyauMode<0)
			open_noyau_modecomm();
		if(mode)
		{
			pwrite (led3, "0", sizeof("0"), 0);
			syslog (LOG_INFO, "Daemon Mode auto");
			if(fdNoyauMode>=0)
				write (fdNoyauMode, "auto", strlen("auto"));
		}
		else
		{
			pwrite (led3, "1", sizeof("1"), 0);
			syslog (LOG_INFO, "Daemon Mode manual");
			if(fdNoyauMode>=0)
				write (fdNoyauMode, "manual", strlen("manual"));
		}
	}
}

static void daemon_periph()
{
// Initialize variable
	int nr;
	int fSw1,fSw2,fSw3;
	int led1,led2;
	uint64_t timersElapsed = 0;
	int i=0;
	int time_counter = 0;

	// Configure LEDs
	led1 = open_led1();
	pwrite (led1, "0", sizeof("0"), 0);

	led2 = open_led2();
	pwrite (led2, "0", sizeof("0"), 0);

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
			syslog (LOG_ERR,"epoll faillure");
			exit (1);
		}
		for(i=0;i<nr;i++)
		{
			// Sw1 event : Up duty cycle
			if(events[i].data.fd == fSw1)
			{
				read (fSw1, &timersElapsed,8);
				sw1_state = !sw1_state;
				time_counter = 0;
				if(sw1_state)
				{
					if(dutyCycle < 100)
						setDutyCycle(dutyCycle+2);
					pwrite (led1, "1", sizeof("1"), 0);
				}
				else
					pwrite (led1, "0", sizeof("0"), 0);
			}
			// Sw2 event : Down duty cycle
			else if(events[i].data.fd == fSw2)
			{
				read (fSw2, &timersElapsed,8);
				sw2_state = !sw2_state;
				time_counter = 0;
				if(sw2_state)
				{
					if(dutyCycle>0)
						setDutyCycle(dutyCycle-2);
					pwrite (led2, "1", sizeof("1"), 0);
				}
				else
					pwrite (led2, "0", sizeof("0"), 0);
			}
			// Sw3 event : Change mode manual/auto
			else if(events[i].data.fd == fSw3)
			{
				read (fSw3, &timersElapsed,8);
				sw3_state = !sw3_state;
				if(sw3_state)
				{
					if(isAuto)
						setMode(false);
					else
						setMode(true);
				}
			}
			// Timer event : Manage long press
			else if(events[i].data.fd == timerfd1)
			{
				read (timerfd1, &timersElapsed,8);
				// Manage long press on sw1 and/or sw3
				if((sw2_state != sw1_state) && (sw2_state| sw1_state))
					time_counter++;
				if(time_counter >= 4000)
				{
					time_counter = 0;
					if(sw2_state && dutyCycle > 0)
						setDutyCycle(dutyCycle-2);
					if(sw1_state && dutyCycle < 100)
						setDutyCycle(dutyCycle+2);
				}
			}
		}
	}
}

static void daemon_IPC()
{
	syslog (LOG_INFO,"daemon IPC-listening");

	//Create fifo
	int code = mkfifo("/tmp/demo6_fifo", 0666);
	if (code == -1) {
		syslog (LOG_ERR,"mkfifo returned an error - file may already exist");
	}

	//Open read end
	int fd = open("/tmp/demo6_fifo", O_RDONLY);
	if (fd == -1) {
		syslog (LOG_ERR,"Cannot open FIFO for read");
		exit(1);
	}
	puts("FIFO OPEN");

	//Read string (upto 255 characters)
	char stringBuffer[256];			//String buffer
	memset(stringBuffer, 0, 256);   //Fill with zeros
	int res;
	char Len;
	while(1) {
		res = read(fd, &Len, 1);
		if(res > 0)
		{
			read(fd, stringBuffer, Len);					//Read string characters
			syslog (LOG_INFO,"Daemon received: %s\n", stringBuffer);
			if(strncmp(stringBuffer, auto_str, 3) == 0)
			{
				setMode(true);
			}
			else if(strncmp(stringBuffer, man_str, 3) == 0)
			{
				setMode(false);
			}
			else if(strncmp(stringBuffer, duty_str, 3) == 0)
			{
				char subbuff[5];
				memcpy( subbuff, &stringBuffer[4], 3);
				subbuff[3] = '\0';
				setDutyCycle(atoi(subbuff));
			}
			memset(stringBuffer, 0, 256);   //Fill with zeros
		}
	}

	close(fd);
	syslog(LOG_INFO,"FIFO Closed");
}

static void catch_signal (int signal)
{
	syslog (LOG_INFO, "signal=%d catched\n", signal);
	signal_catched++;
}

static void fork_process()
{
	pid_t pid = fork();
	switch (pid) {
		case  0: break; // child process has been created
		case -1: syslog (LOG_ERR, "ERROR while forking"); exit (1); break;
		default: exit(0);  // exit parent process with success
	}
}

int main(int argc, char* argv[])
{
	UNUSED(argc); UNUSED(argv);

	int n = 2;
	pid_t pids[2];
	int i;

	// 1. fork off the parent process
	fork_process();

	// 2. create new session
	if (setsid() == -1) {
		syslog (LOG_ERR, "ERROR while creating new session");
		exit (1);
	}

	//fork_process();

	// 4. capture all required signals
	struct sigaction act = {.sa_handler = catch_signal,};
	sigaction (SIGHUP,  &act, NULL);  //  1 - hangup
	sigaction (SIGINT,  &act, NULL);  //  2 - terminal interrupt
	sigaction (SIGQUIT, &act, NULL);  //  3 - terminal quit
	sigaction (SIGABRT, &act, NULL);  //  6 - abort
	sigaction (SIGTERM, &act, NULL);  // 15 - termination
	sigaction (SIGTSTP, &act, NULL);  // 19 - terminal stop signal

	// 5. update file mode creation mask
	umask(0027);

	// 6. change working directory to appropriate place
	if (chdir ("/") == -1) {
		syslog (LOG_ERR, "ERROR while changing to working directory");
		exit (1);
	}

	// 7. close all open file descriptors
	for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--) {
		close (fd);
	}

	// 8. redirect stdin, stdout and stderr to /dev/null
	if (open ("/dev/null", O_RDWR) != STDIN_FILENO) {
		syslog (LOG_ERR, "ERROR while opening '/dev/null' for stdin");
		exit (1);
	}
	if (dup2 (STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
		syslog (LOG_ERR, "ERROR while opening '/dev/null' for stdout");
		exit (1);
	}
	if (dup2 (STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
		syslog (LOG_ERR, "ERROR while opening '/dev/null' for stderr");
		exit (1);
	}

	// 9. option: open syslog for message logging
	openlog (NULL, LOG_NDELAY | LOG_PID, LOG_DAEMON);
	syslog (LOG_INFO, "Daemon has started...");

	// 10. option: get effective user and group id for appropriate's one
	struct passwd* pwd = getpwnam ("root");
	if (pwd == 0) {
		syslog (LOG_ERR, "ERROR while reading daemon password file entry");
		exit (1);
	}

	// 11. option: change root directory
	if (chroot (".") == -1) {
		syslog (LOG_ERR, "ERROR while changing to new root directory");
		exit (1);
	}

	// 12. option: change effective user and group id for appropriate's one
	if (setegid (pwd->pw_gid) == -1) {
		syslog (LOG_ERR, "ERROR while setting new effective group id");
		exit (1);
	}
	if (seteuid (pwd->pw_uid) == -1) {
		syslog (LOG_ERR, "ERROR while setting new effective user id");
		exit (1);
	}

	// OPEN SYSFS Files
	open_noyau_dutycomm();
	open_noyau_modecomm();

	led3 = open_led3();
	pwrite (led3, "0", sizeof("0"), 0);

	setMode(true);
	setDutyCycle(50);	

	/* Start children. */
	for (i = 0; i < n; ++i) {
		if ((pids[i] = fork()) < 0) {
			syslog(LOG_ERR,"fork");
			abort();
		} else if (pids[i] == 0 && i == 0) {

			// 13. implement daemon body...
			syslog (LOG_INFO, "Daemon body started...");
			daemon_IPC();
			exit(0);
		}
		else if (pids[i] == 0 && i == 1){
			daemon_periph();
			exit(0);
		}
	}

	syslog (LOG_INFO, "daemon stopped. Number of signals catched=%d\n", signal_catched);
	closelog();

	return 0;
}

