////////////////////////////////////////////////////////////////////////////////
// 15-745 Project: MemoryAccess.cpp
// Group: jarulraj, akalia, junwoop
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <time.h>
#include <Eigen/Dense>
#include <iostream>

// Timing
struct timespec tstart={0,0}, tend={0,0};

typedef Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Matrix;

void StartTimer() {
    clock_gettime(CLOCK_MONOTONIC, &tstart);
};

void StopTimer() {
    clock_gettime(CLOCK_MONOTONIC, &tend);

    printf("Duration :: %.5f seconds\n",((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
            ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
}

// Initialize the matrix
void InitMatrix(Matrix *a) {
    for (size_t row_itr = 0; row_itr < a->rows(); row_itr++ ) {
        for (size_t col_itr = 0; col_itr < a->cols(); col_itr++ ) {
            a->coeffRef(row_itr, col_itr) = row_itr + col_itr;
        }
    }
}

// Sum a row
int SumRow(Matrix *matrix, int row_id) {

    int sum = matrix->row(row_id).sum();
    printf("sum of row : %d :: %d \n", row_id, sum);
    return sum;
}

// Sum a column
int SumColumn(Matrix *matrix, int col_id) {
    int sum = matrix->col(col_id).sum();

    printf("sum of col : %d :: %d \n", col_id, sum);
    return sum;
}

#define SIZE 16

// Tester
int main () {

    Matrix a(1024 * SIZE, 1024 * SIZE); 

    InitMatrix(&a);
    //PrintMatrix(a);
    std::cout << "init\n";

    StartTimer();
    SumRow(&a, 2);
    StopTimer();

    std::cout << "sum row\n";
    StartTimer();
    SumColumn(&a, 3);
    StopTimer();

    return 0;
}


