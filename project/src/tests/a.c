#include <stdio.h>
#include <stdlib.h>


void foo(long n, long m, int A[n][m]) {

    for (long i = 0; i < n; i++)
        for (long j = 0; j < m; j++){
                A[i][j] = 0;
                A[j][i] = 0;
        }

}
