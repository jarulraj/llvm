#include <stdio.h>

unsigned triangularNumber(unsigned n);

const char *suffix[] = {" ","st","nd","rd"};

int abc(int argc, char *argv[]) {
    int n = 0,m;
    unsigned t;

    if ((argc < 2) ||
            (sscanf(argv[1],"%d",&n) != 1) ||
            (n <= 0)) {
        printf("\nusage %s <n>\n",argv[0]);
        printf(" where <n> is a positive integer\n");
        return -1;
    }
    t = (int)(n * (((float)n + 1.0) / 2.0));
    t *= 3;
    if (t == triangularNumber(n)) {
        m = n % 10;
        printf("\nThe %d%s triangular number is %d\n",
                n,
                ((m > 0) && (m < 4)) ? suffix[m] : "th",
                t);
    } else {
        printf("\nerror\n");
    }
    return 0;
}

unsigned triangularNumber(unsigned n) {
    if (n == 1) return 1;
    return n + triangularNumber(n-1);
}

void heapsort(int arr[], unsigned int N)
{
    unsigned int n = N, i = n/2, parent, child;
    int t;

    for (;;) { /* Loops until arr is sorted */
        if (i > 0) { /* First stage - Sorting the heap */
            i--;           /* Save its index to i */
            t = arr[i];    /* Save parent value to t */
        } else {     /* Second stage - Extracting elements in-place */
            n--;           /* Make the new heap smaller */
            if (n == 0) return; /* When the heap is empty, we are done */
            t = arr[n];    /* Save last value (it will be overwritten) */
            arr[n] = arr[0]; /* Save largest value at the end of arr */
        }

        parent = i; /* We will start pushing down t from parent */
        child = i*2 + 1; /* parent's left child */

        /* Sift operation - pushing the value of t down the heap */
        while (child < n) {
            if (child + 1 < n  &&  arr[child + 1] > arr[child]) {
                child++; /* Choose the largest child */
            }
            if (arr[child] > t) { /* If any child is bigger than the parent */
                arr[parent] = arr[child]; /* Move the largest child up */
                parent = child; /* Move parent pointer to this child */
                //child = parent*2-1; /* Find the next child */
                child = parent*2+1; /* the previous line is wrong*/
            } else {
                break; /* t's place is found */
            }
        }
        arr[parent] = t; /* We save t in the heap */
    }
} 

#include "stdio.h"
#include "stdlib.h"

#define SCALE 10000
#define ARRINIT 2000

void pi_digits(int digits) {
    int carry = 0;
    int arr[digits + 1];
    for (int i = 0; i <= digits; ++i)
        arr[i] = ARRINIT;
    for (int i = digits; i > 0; i-= 14) {
        int sum = 0;
        for (int j = i; j > 0; --j) {
            sum = sum * j + SCALE * arr[j];
            arr[j] = sum % (j * 2 - 1);
            sum /= j * 2 - 1;
        }
        printf("%04d", carry + sum / SCALE);
        carry = sum % SCALE;
    }
}

int main(int argc, char** argv) {
    int n = argc == 2 ? atoi(argv[1]) : 100;
    pi_digits(n);

    return 0;
}
