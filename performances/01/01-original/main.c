#include <stdint.h>

#define SIZE 5000

static int32_t array[SIZE][SIZE];

int main (void)
{
    int i, j, k;

    for (k = 0; k < 50; k++)
    {
        for (i = 0; i < SIZE; i++)
        {
            for (j = 0; j < SIZE; j++)
            {
                array[j][i]++;
            }
        }
    }
    return 0;
}

