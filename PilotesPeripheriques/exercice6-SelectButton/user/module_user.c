#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/mman.h>

unsigned int *addr;

int main (int ac, char **av)
{
	int fd;
	fd_set fds;
	int interrupt_count = 0;

	fd = open (av[1], O_RDWR);
	if (fd < 0) {
		perror ("open");
		exit (1);
	}
	printf ("file %s open\n",av[1]);

	while(interrupt_count < 5)
	{
		FD_ZERO(& fds);
		FD_SET(fd, & fds);

		/*if (select(fd+1, NULL, NULL, &fds, NULL) < 0) {
		    perror("select");
		    break;
		}*/

		if (select(fd+1, &fds, NULL, NULL, NULL) < 0) {
		    perror("select");
		    break;
		}
		interrupt_count++;
		printf("Interrupt %d\n",interrupt_count);
	}

	close (fd);
	printf ("file %s close\n",av[1]);

	return 0;
}
