#include <stdio.h>
#include <stdlib.h>

#define SIZE 32768

static int compare (const void * a, const void * b)
{
	return *(short*)a - *(short*)b;
}

int main()
{
 	// generate data
	short data[SIZE];
	for (int i = 0; i < SIZE; i++) 
	{
		data[i] = rand() % 256;
	}

	qsort(data, SIZE, sizeof(data[0]), compare);

	long long sum = 0;
	for (int j = 0; j < 100000; j++) 
	{
		for (int i = 0; i < SIZE; i++) 
		{
			if (data[i] >= 128) 
			{
				sum += data[i];
			}
		}
	}
	printf ("sum = %lld\n", sum);
}
