#include <stdio.h>
#include <stdlib.h>

void foo(long n, long m)  {
    __attribute__((annotate("hey, this is important"))) int A[n][m];
 
    struct key{
        char a;
        char b;
        char c;
    };

    __attribute__((annotate("hey, keys"))) struct key keys[100]; 

    char x;
    for (long i = 0; i < n; i++) {
        for (long j = 0; j < m; j++){
            A[i][j] = 0;
            A[j][i] = 0;

            x = keys[i].a;
            keys[i].b = x;
        }
    }

}
