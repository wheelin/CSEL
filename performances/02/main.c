#include <stdio.h>
#include <stdlib.h>

#define SIZE 32768

int main()
{
 	// generate data
	short data[SIZE];
	for (int i = 0; i < SIZE; i++) 
	{
		data[i] = rand() % 256;
	}

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
