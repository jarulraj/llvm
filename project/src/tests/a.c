#include <stdio.h>
#include <stdlib.h>

void foo(long n, long m)  {
    __attribute__((annotate("hey, this is important"))) char A[128][128];

    struct key{
        char data[8];
    };

    size_t len = 8;
    __attribute__((annotate("hey, keys"))) struct key keys[len]; 

    char x;
    int i, j;

    for (i = 0; i < len ; i++) {
        for (j = 0; j < len ; j++) {
            x = keys[i].data[j];
            x = keys[j].data[i];
        }
    }

}
