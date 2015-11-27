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
static struct sigaction act = {.sa_handler = catch_signal,};

int main (int argc, char** argv)
{
	int i, err;
    for(i = 1; i <= 30; i++)
    {
    	if(i != SIGKILL && i != SIGSTOP)
    	{
    		err = sigaction(i, &act, NULL);
    		if(err == -1)
    		{
    			printf("Cannot handle signal : %d\n", i);
    			return -1;
    		}
    	}
    }

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
    	close(fd[0]);
    	children_code(fd[1]);

    }
    else if(pid > 0)
    {
    	/* parent's code */
    	printf("%s\n", "In the parent process\n");
    	close(fd[1]);
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
	char msg[50];
	while(1)
	{
		printf("Enter a msg to send to the parent : ");
		scanf("%s", msg);
		printf("%s will be written to the parent.\n", msg);
		write(fd_in_pipe, msg, sizeof(msg));
		if(strcmp(msg, "exit") == 0)
			exit(0);
	}
}

void parent_code(int fd_out_pipe)
{
	char msg[50];
	int numB;
	while(1)
	{
		numB = read(fd_out_pipe, msg, sizeof(msg));
		if(numB > 0)
		{
			printf("Received %d from the children. String is : %s\n", numB, msg);
			if(strcmp(msg, "exit") == 0)
				exit(0);
		}

		else sleep(1);
	}
}
