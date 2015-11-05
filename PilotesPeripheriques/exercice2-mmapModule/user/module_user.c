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
	char buf[64];
	int n,fd;
	fd = open (av[1], O_RDWR);
	if (fd < 0) {
		perror ("open");
		exit (1);
	}
	printf ("file %s open\n",av[1]);

	addr = mmap(0, 0x100, PROT_READ, MAP_SHARED, fd, 0x10000000);
	if (addr == MAP_FAILED)
	{
		printf("mmap error");
		exit(1);
	}
	// Read memory id
	printf("Physical memory: Bit 31..12 : product id=0x%05x\n",(addr[0]>>12)&0x7ffff);
	printf("Physical memory: Bit 11..8  : package id=0x%01x\n",(addr[0]>>8)&0xf);
	printf("Physical memory: Bit 7..4   : major revision=0x%01x\n",(addr[0]>>4)&0xf);
	printf("Physical memory: Bit 3..0   : minor revision=0x%01x\n\n",(addr[0])&0xf);

	printf("Virtual memory: Bit 31..12 : product id=0x%05x\n",(addr[1]>>12)&0x7ffff);
	printf("Virtual memory: Bit 11..8  : package id=0x%01x\n",(addr[1]>>8)&0xf);
	printf("Virtual memory: Bit 7..4   : major revision=0x%01x\n",(addr[1]>>4)&0xf);
	printf("Virtual memory: Bit 3..0   : minor revision=0x%01x\n",(addr[1])&0xf);

	close (fd);
	printf ("file %s close\n",av[1]);

	return 0;
}
