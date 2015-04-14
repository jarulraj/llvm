#include <stdio.h>
#include <stdlib.h>

int foo(long n, long m) {

    __attribute__((annotate("hey, this is important"))) char A[32][64];

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++){
            A[i][j] = 0;
            A[j][i] = 0;
        }
    }

    return 0;
}
