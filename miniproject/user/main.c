
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(void) {
	puts("User app");
	char str[80];

	//Write each string received
	while(1){
		scanf("%79s",str);
		//Open fifo for write
		int fd = open("/tmp/demo6_fifo", O_WRONLY);
		if (fd ==  -1) {
			perror("Cannot open fifo");
			return EXIT_FAILURE;
		}
		write(fd, &str, 1); //Send string length
		write(fd, str, strlen(str)); 	//Send string characters
		memset(str, 0, 80);   //Fill with zeros
		close(fd);
	}	
	return EXIT_SUCCESS;
}
