#include <stdio.h>
#include <stdlib.h>
int main() {


    int i;
    //__attribute__((annotate("matrix annotation"))) char a[10];
    char a[10];
    __attribute__((annotate("hey, this is important"))) char b[16][32][64];

    int j;
    int sum = 0;
    int m = 10;
    int n = 30;

    for(i = 0 ; i < 100 ; i++) {
        j = i + 2;
        //a[j] = i * j;
    }

    for (int i = 0; i < 16; i ++) {
        for (int j = 0; j < 32; j++) {
			for(int k = 0; k < 64; k ++) {
				b[i][j][k] = 0;
				b[i][j][0] = 0;
				b[i][0][k] = 0;
				b[0][j][k] = 0;
				b[0][0][k] = 0;
				b[i][0][0] = 0;
			}
        }
	}

    printf("sum : %d\n", sum);

    return 0;
}
