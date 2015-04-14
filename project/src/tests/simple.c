#include <stdio.h>
#include <stdlib.h>
int main() {


    int i;
    //__attribute__((annotate("matrix annotation"))) char a[10];
    char a[10];
    __attribute__((annotate("hey, this is important"))) char b[32][64];

    int j;
    int sum = 0;
    int m = 10;
    int n = 30;

    for(i = 0 ; i < 100 ; i++) {
        j = i + 2;
        //a[j] = i * j;
    }

    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++){
            b[i][j+5] = 0;
            //b[j][i] = 0;

            //a[i*n+j] = 0;
            //a[j*n+i] = 0;
        }
    printf("sum : %d\n", sum);

    return 0;
}
