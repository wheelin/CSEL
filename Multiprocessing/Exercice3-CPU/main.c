#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
    
void catch_signal(int signo);
void children_code(int fd_in_pipe);
void parent_code(int fd_out_pipe);

static int fd[2];

int main (int argc, char** argv)
{
	int err;

    err = pipe(fd);
    if(err == -1)
	{
		return -1;
	}
	printf("%s\n", "In the main process.\n");
    pid_t pid = fork();
    if(pid == 0)
    {
    	/* children's code*/
    	printf("%s\n", "In the children process.\n");
	children_code(fd[1]);
    }
    else if(pid > 0)
    {
    	/* parent's code */
    	printf("%s\n", "In the parent process\n");
	parent_code(fd[0]);
    }
    else
    {
    	/* error */
    	return -1;
    }
    printf("\n");
    return 0;
}

void catch_signal(int signo)
{
	printf("Received signal is : %s\n", strsignal(signo));
}

void children_code(int fd_in_pipe)
{
	while(1){}
}

void parent_code(int fd_out_pipe)
{
	while(1){}
}
