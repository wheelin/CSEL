#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define SIZE 5000

static int32_t array[SIZE][SIZE];

int main (void)
{
    int i, j, k;
    struct timespec t1;
    struct timespec t2;

    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (k = 0; k < 50; k++)
    {
        for (i = 0; i < SIZE; i++)
        {
            for (j = 0; j < SIZE; j++)
            {
                //array[j][i]++;
		array[i][j]++;
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
    long long int delta_t = ((long long)(t2.tv_sec-t1.tv_sec) * 1000000000
				+(t2.tv_nsec-t1.tv_nsec))/1000000;
    printf("elapsed time: %lld [ms]\n", delta_t);
    return 0;
}

