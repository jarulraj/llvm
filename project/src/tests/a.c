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
    int i, j, k;

    for (i = 0; i < len ; i++) {
        for (j = 0; j < len ; j++) {
            x = keys[i].data[j];
            x = keys[j].data[i];
            x = keys[i].data[j];
        }
    }

    for(k = 0 ; k < 100 ; k++) { 
        for (i = 0; i < n; i+=2) {
            for (j = 0; j < m; j++){
                A[i][j] = 0;
                A[j][i] = 0;

                x = keys[i].data[0];
                keys[i].data[1] = x;
            }
        }
    }


}
