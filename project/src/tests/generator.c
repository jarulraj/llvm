////////////////////////////////////////////////////////////////////////////////
// 15-745 Project: Generator.c
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
    int *data;
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
    ret->data = (int *) malloc(sizeof (int) * num_rows * num_cols);

    return ret;
}

// Initialize the matrix
void InitMatrix(struct Matrix *matrix) {
    int *data = matrix->data;
    assert(data);
    int row_itr, col_itr;

    size_t num_rows = matrix->num_rows;
    size_t num_cols = matrix->num_cols;

    for (row_itr = 0; row_itr < num_rows; row_itr++ ) {
        for (col_itr = 0; col_itr < num_cols; col_itr++ ) {
            data[row_itr * num_cols + col_itr] = row_itr + col_itr;
        }
    }
}

// Display the matrix
void PrintMatrix(struct Matrix *matrix) {
    int *data = matrix->data;
    int row_itr, col_itr;

    size_t num_rows = matrix->num_rows;
    size_t num_cols = matrix->num_cols;

    for (row_itr = 0; row_itr < num_rows; row_itr++ ) {
        for (col_itr = 0; col_itr < num_cols; col_itr++ ) {
            printf("%d ", data[row_itr * num_cols + col_itr]);
        }
        printf("\n");
    }
}

// Sum a row
int SumRow(struct Matrix *matrix, int row_id) {
    int *data = matrix->data;
    size_t num_cols = matrix->num_cols;
    int col_itr;

    int sum = 0;
    for (col_itr = 0; col_itr < num_cols; col_itr++ ) {
        sum += data[row_id * num_cols + col_itr];
    }

    printf("sum of row : %d :: %d \n", row_id, sum);
    return sum;
}

// Sum a column
int SumColumn(struct Matrix *matrix, int col_id) {
    int *data = matrix->data;
    size_t num_rows = matrix->num_rows;
    size_t num_cols = matrix->num_cols;
    int row_itr;

    int sum = 0;

    for (row_itr = 0; row_itr < num_rows; row_itr++ ) {
        sum += data[row_itr * num_cols + col_id];
    }

    printf("sum of col : %d :: %d \n", col_id, sum);
    return sum;
}

void AccessMatrix(struct Matrix *matrix, int pattern_id, double rd_wr_ratio){
    int *data = matrix->data;
    size_t num_rows = matrix->num_rows;
    size_t num_cols = matrix->num_cols;

}


#define SIZE 16

// Tester
int main (int argc, char *argv[]) {

    struct Matrix *mat;

    if(argc != 3){
        fprintf(stderr, "Usage :: ./generator <pattern_id (0-7)> <rd-wr ratio (0-1)> \n");
        return -1;
    }
    
    fprintf(stdout, "initializing square matrix with dimension : %d \n", 1024*SIZE);

    mat = AllocateMatrix(1024 * SIZE, 1024 * SIZE);
    InitMatrix(mat);

    int pattern_id = atoi(argv[1]);
    double rd_wr_ratio = atof(argv[2]);

    fprintf(stdout, "pattern id : %d rd_wr ratio : %.2f \n", pattern_id, rd_wr_ratio);

    StartTimer();
    AccessMatrix(mat, pattern_id, rd_wr_ratio);
    StopTimer();

    return 0;
}
