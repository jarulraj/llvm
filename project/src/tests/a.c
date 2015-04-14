#include <stdio.h>
#include <stdlib.h>


void foo(long n, long m)  {
    __attribute__((annotate("hey, this is important"))) int A[n][m];

    for (long i = 0; i < n; i++) {
        for (long j = 0; j < m; j++){
            A[i][j] = 0;
            A[j][i] = 0;

            A[2*i+j][i-5*j] = 1;
        }
    }

}
