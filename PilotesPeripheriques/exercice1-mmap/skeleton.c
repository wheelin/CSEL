#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>

static volatile uint32_t *chipid = 0;

int main()
{
	int fd;

	fd = open("/dev/mem", O_RDWR);
	if (fd == -1) {
		printf("Error while trying to open...\n");
		return -EFAULT;
	}

	chipid = mmap(0, 0x100, PROT_READ, MAP_SHARED, fd, 0x10000000);
	if (chipid == (void*)-1) {
		close(fd);
		printf("Error while trying to mmap...\n");
		return -EFAULT;
	}
    
    	// Read memory id
	printf("uP register: Bit 31..12 : product id=0x%05x\n",(*chipid>>12)&0x7ffff);
	printf("uP register: Bit 11..8  : package id=0x%01x\n",(*chipid>>8)&0xf);
	printf("uP register: Bit 7..4   : major revision=0x%01x\n",(*chipid>>4)&0xf);
	printf("uP register: Bit 3..0   : minor revision=0x%01x\n",(*chipid)&0xf);

	munmap((void*)chipid, 0x100);
	close(fd);
	return 0;
}

