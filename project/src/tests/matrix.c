////////////////////////////////////////////////////////////////////////////////
// 15-745 Project: MemoryAccess.cpp
// Group: jarulraj, akalia, junwoop
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <time.h>

/* Simple matrix ADT */
struct Matrix {
    size_t num_rows;
    size_t num_cols;

    // actual data
    int **data;
};

// Timing
struct timespec tstart={0,0}, tend={0,0};

void StartTimer() {
    clock_gettime(CLOCK_MONOTONIC, &tstart);
};

void StopTimer() {
    clock_gettime(CLOCK_MONOTONIC, &tend);

    printf("Duration :: %.5f seconds\n",((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
            ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
}


// Allocate storage
struct Matrix *AllocateMatrix(size_t num_rows, size_t num_cols) {

    struct Matrix *ret = (struct Matrix *) malloc(sizeof(struct Matrix));
    int row_itr;

    ret->num_rows = num_rows;
    ret->num_cols = num_cols;

    // allocate space
    ret->data = (int **) malloc(sizeof (int *) * num_rows);
    if(ret->data) {

        for (row_itr = 0; row_itr < num_rows; row_itr++) {
            ret->data[row_itr] = (int *) malloc(sizeof (int) * num_cols);
        }
    }

    return ret;
}

// Initialize the matrix
void InitMatrix(struct Matrix *matrix) {
    int **data = matrix->data;
    assert(data);
    int row_itr, col_itr;

    size_t num_rows = matrix->num_rows;
    size_t num_cols = matrix->num_cols;

    for (row_itr = 0; row_itr < num_rows; row_itr++ ) {
        for (col_itr = 0; col_itr < num_cols; col_itr++ ) {
            data[row_itr][col_itr] = row_itr + col_itr;
        }
    }
}

// Display the matrix
void PrintMatrix(struct Matrix *matrix) {
    int **data = matrix->data;
    int row_itr, col_itr;

    size_t num_rows = matrix->num_rows;
    size_t num_cols = matrix->num_cols;

    for (row_itr = 0; row_itr < num_rows; row_itr++ ) {
        for (col_itr = 0; col_itr < num_cols; col_itr++ ) {
            printf("%d ", data[row_itr][col_itr]);
        }
        printf("\n");
    }
}

// Sum a row
int SumRow(struct Matrix *matrix, int row_id) {
    int **data = matrix->data;
    size_t num_cols = matrix->num_cols;
    int col_itr;

    int sum = 0;
    for (col_itr = 0; col_itr < num_cols; col_itr++ ) {
        sum += data[row_id][col_itr];
    }

    printf("sum of row : %d :: %d \n", row_id, sum);
    return sum;
}

// Sum a column
int SumColumn(struct Matrix *matrix, int col_id) {
    __attribute__((annotate("matrix annotation")))int **data = matrix->data;
    size_t num_rows = matrix->num_rows;
    int row_itr;

    int sum = 0;

    for (row_itr = 0; row_itr < num_rows; row_itr++ ) {
        sum += data[row_itr][col_id];
    }

    printf("sum of col : %d :: %d \n", col_id, sum);
    return sum;
}

#define SIZE 16

// Tester
int main () {

    struct Matrix *a;

    a = AllocateMatrix(1024 * SIZE, 1024 * SIZE);
    InitMatrix(a);
    //PrintMatrix(a);

    StartTimer();
    SumRow(a, 2);
    StopTimer();

    StartTimer();
    SumColumn(a, 3);
    StopTimer();

    return 0;
}


