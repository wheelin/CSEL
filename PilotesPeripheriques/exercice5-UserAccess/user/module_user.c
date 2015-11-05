#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <fcntl.h>

int main (int ac, char **av)
{
	char buf[64];
	int n, fd;
	fd = open (av[1], O_RDWR);
	if (fd < 0) {
		perror ("open");
		exit (1);
	}
	printf ("file %s open\n",av[1]);

	n = write (fd, av[2], strlen(av[2]));
	printf ("write %s\n",av[2]);

	read (fd, buf, n);
	printf ("read %s\n",buf);

	close (fd);
	printf ("file %s close\n",av[1]);

	return 0;
}
