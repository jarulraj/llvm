#include <stdio.h>
#include <stdlib.h>
int main() {


    int i;
    __attribute__((annotate("matrix annotation"))) char a[100];
    __attribute__((annotate("hey, this is important"))) int m[100][100];
    __attribute__((annotate("hey, this is important"))) int m2[100][100][5];


    int j;
    int sum = 0;
    int n = 23;

    for(i = 0 ; i < 100 ; i++) {
        j = i + 2;
        //a[j] = i * j;
    }

    for (int i = 0; i < n; i++)
        for (int j = 0; j < 100; j++){
            for (int k = 0; k < 5; k++) {
                m2[i][j][k] = 0;
                m2[j][i][k] = 0;
            }
            m[i][j] = 0;
            m[j][i] = 0;

            a[i*n+j] = 0;
            a[j*n+i] = 0;

            //a[2*i+23] = 0;
        }
    printf("sum : %d\n", sum);

    return 0;
}
