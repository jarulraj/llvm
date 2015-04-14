#include <stdio.h>
#include <stdlib.h>

int main() {

    int i, j;
    int m = 10;
    int n = 30;

    __attribute__((annotate("hey, this is important"))) char b[32][64];

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++){
            b[i][j] = 0;
        }
    }

    return 0;
}
