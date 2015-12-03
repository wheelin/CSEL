#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define ALLOC_LEN 50

uint8_t * pointers[ALLOC_LEN];

int main (int argc, char** argv)
{
	int i;
	for (i = 0; i < ALLOC_LEN; ++i)
	{
		pointers[i] = (uint8_t*) malloc(1000000);
		if(pointers[i] == NULL) return -1;
		if((i%10) == 0 && i != 0) printf("Ten more megas allocated\n");
		memset(pointers[i], 0, 1000000);
	}

	printf("%s\n", "Press enter to continue...\n");
	getchar();

	for (i = 0; i < ALLOC_LEN; ++i)
	{
		free(pointers[i]);
		if((i%10) == 0) printf("Ten more megas freed\n");
	}
    return 0;
}


