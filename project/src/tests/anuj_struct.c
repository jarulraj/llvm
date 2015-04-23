#include <stdio.h>
#include <stdlib.h>

#define ARRAY_CAP 256

void foo(long n, long m)
{
    struct key_t {
    	long long data[2];
    };

    __attribute__((annotate("hey, keys"))) struct key_t keys[ARRAY_CAP]; 

    long long sum = 0;
    int i, j;

	/**< Access with stride = 8B */
    for (i = 0; i < ARRAY_CAP ; i++) {
        for (j = 0; j < 2 ; j++) {
            sum += keys[i].data[j];
        }
    }

	/**< Access with stride = 8B */
    for (i = 0; i < ARRAY_CAP ; i++) {
		sum += keys[i].data[0];
	}

}
