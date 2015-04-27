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

void AccessMatrix(struct Matrix *matrix, int pattern_id, double rd_wr_ratio, int scale){
    int *data = matrix->data;
    size_t num_rows = matrix->num_rows;
    size_t num_cols = matrix->num_cols;
    size_t num_tiles = (num_rows * num_cols)/ (scale * scale);

    int row_itr, col_itr, tile_itr;
    double rd_wr;
    double sum = 0;
    int row_offset, col_offset;

    for(tile_itr = 0 ; tile_itr < num_tiles ; tile_itr++) {
        int *tile = data + (tile_itr * scale * scale);
        rd_wr= ((double)rand()/(double)RAND_MAX);

        // WRITES
        if(rd_wr < rd_wr_ratio) {
            switch(pattern_id){
                case 0:
                    for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                        for(col_itr = 0 ; col_itr < scale ; col_itr++){
                            tile[row_itr*8 + col_itr] = 1;
                        }
                    }
                    break;

                case 1:
                    for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                        row_offset = row_itr%2;
                        for(col_itr = row_offset ; col_itr < scale ; col_itr+=2){
                            tile[(row_itr-row_offset)*scale+(col_itr+row_offset)] = 1;
                            tile[(row_itr-row_offset+1)*scale+(col_itr+row_offset)] = 1;
                        }
                    } 
                    break;

                case 2:
                    for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                        row_offset = row_itr%4;
                        for(col_itr = row_offset/2 ; col_itr < scale ; col_itr+=4){
                            tile[(row_itr-row_offset)*scale+(col_itr)] = 1;
                            tile[(row_itr-row_offset+2)*scale+(col_itr+1)] = 1;
                            tile[(row_itr-row_offset)*scale+(col_itr+(scale/2))] = 1;
                            tile[(row_itr-row_offset+2)*scale+(col_itr+(scale/2)+1)] = 1;
                        }
                    }
                    break;

                case 3:
                    for(col_itr = 0 ; col_itr < scale ; col_itr++) {
                        col_offset = col_itr%(scale/2);
                        row_offset = col_itr - col_itr%(scale/2);
                        for(row_itr = 0 ; row_itr < (scale/2) ; row_itr++){
                            tile[(col_offset)*scale+(row_offset+row_itr)] = 1;
                            tile[(col_offset+(scale/2))*scale+(row_offset+row_itr)] = 1;
                        }
                    }
                    break;

                case 4:
                    for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                        row_offset = row_itr%(scale/2);
                        row_offset = row_itr - row_itr%(scale/2);
                        for(col_itr = 0 ; col_itr < (scale/2) ; col_itr++){
                            tile[(row_offset)*scale+(col_offset+col_itr)] = 1;
                            tile[(row_offset+(scale/2))*scale+(col_offset+col_itr)] = 1;
                        }
                    }
                    break;
                
                case 5:
                    for(col_itr = 0 ; col_itr < scale ; col_itr++) {
                        col_offset = col_itr%4;
                        for(row_itr = col_offset/2 ; row_itr < scale ; row_itr+=4){
                            tile[(col_itr-col_offset)*scale+(row_itr)] = 1;
                            tile[(col_itr-col_offset+2)*scale+(row_itr+1)] = 1;
                            tile[(col_itr-col_offset)*scale+(row_itr+(scale/2))] = 1;
                            tile[(col_itr-col_offset+2)*scale+(row_itr+(scale/2)+1)] = 1;
                        }
                    }
                    break;

                case 6:
                    for(col_itr = 0 ; col_itr < scale ; col_itr++) {
                        col_offset = col_itr%2;
                        for(row_itr = col_offset ; row_itr < scale ; row_itr+=2){
                            tile[(col_itr-col_offset)*scale+(row_itr+col_offset)] = 1;
                            tile[(col_itr-col_offset+1)*scale+(row_itr+col_offset)] = 1;
                        }
                    }
                    break;

                case 7:
                    for(col_itr = 0 ; col_itr < scale ; col_itr++){
                        for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                            tile[row_itr*scale + col_itr] = 1;
                        }
                    }
                    break;

                default:
                    fprintf(stderr, "Unsupported pattern id : %d\n", pattern_id);
                    exit(-1);
            };

        }
        // READS
        else {

            switch(pattern_id){
                case 0:
                    for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                        for(col_itr = 0 ; col_itr < scale ; col_itr++){
                            sum += tile[row_itr*scale + col_itr];
                        }
                    }
                    break;

                case 1:
                    for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                        row_offset = row_itr%2;
                        for(col_itr = row_offset ; col_itr < scale ; col_itr+=2){
                            sum += tile[(row_itr-row_offset)*scale+(col_itr+row_offset)];
                            sum += tile[(row_itr-row_offset+1)*scale+(col_itr+row_offset)];
                        }
                    }
                    break;
                
                case 2:
                    for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                        row_offset = row_itr%4;
                        for(col_itr = row_offset/2 ; col_itr < scale ; col_itr+=4){
                            sum += tile[(row_itr-row_offset)*scale+(col_itr)];
                            sum += tile[(row_itr-row_offset+2)*scale+(col_itr+1)];
                            sum += tile[(row_itr-row_offset)*scale+(col_itr+(scale/2))];
                            sum += tile[(row_itr-row_offset+2)*scale+(col_itr+(scale/2)+1)];
                        }
                    }
                    break;

                case 3:
                    for(col_itr = 0 ; col_itr < scale ; col_itr++) {
                        col_offset = col_itr%(scale/2);
                        row_offset = col_itr - col_itr%(scale/2);
                        for(row_itr = 0 ; row_itr < scale/2 ; row_itr++){
                            sum += tile[(col_offset)*scale+(row_offset+row_itr)];
                            sum += tile[(col_offset+4)*scale+(row_offset+row_itr)];
                        }
                    }
                    break;

                case 4:
                    for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                        row_offset = row_itr%(scale/2);
                        col_offset = row_itr - row_itr%(scale/2);
                        for(col_itr = 0 ; col_itr < (scale/2) ; col_itr++){
                            sum += tile[(row_offset)*scale+(col_offset+col_itr)];
                            sum += tile[(row_offset+(scale/2))*scale+(col_offset+col_itr)];
                        }
                    }
                    break;
                
                case 5:
                    for(col_itr = 0 ; col_itr < scale ; col_itr++) {
                        col_offset = col_itr%4;
                        for(row_itr = col_offset/2 ; row_itr < scale ; row_itr+=4){
                            sum += tile[(col_itr-col_offset)*scale+(row_itr)];
                            sum += tile[(col_itr-col_offset+2)*scale+(row_itr+1)];
                            sum += tile[(col_itr-col_offset)*scale+(row_itr+(scale/2))];
                            sum += tile[(col_itr-col_offset+2)*scale+(row_itr+(scale/2)+1)];
                        }
                    }
                    break;

                case 6:
                    for(col_itr = 0 ; col_itr < scale ; col_itr++) {
                        col_offset = col_itr%2;
                        for(row_itr = col_offset ; row_itr < scale ; row_itr+=2){
                            sum += tile[(col_itr-col_offset)*scale+(row_itr+col_offset)];
                            sum += tile[(col_itr-col_offset+1)*scale+(row_itr+col_offset)];
                        }
                    }
                    break;

                case 7:
                    for(col_itr = 0 ; col_itr < scale ; col_itr++){
                        for(row_itr = 0 ; row_itr < scale ; row_itr++) {
                            sum += tile[row_itr*scale + col_itr];
                        }
                    }
                    break;

                default:
                    fprintf(stderr, "Unsupported pattern id : %d\n", pattern_id);
                    exit(-1);
            };

        }
    }

}


#define SIZE 16

// Tester
int main (int argc, char *argv[]) {

    struct Matrix *mat;

    if(argc != 4){
        fprintf(stderr, "Usage :: ./generator <pattern_id (0-7)> <rd-wr ratio (0-1)> scale\n");
        return -1;
    }

    int mat_size = SIZE;
    fprintf(stdout, "initializing square matrix with dimension : %d \n", 1024*mat_size);

    mat = AllocateMatrix(64*mat_size, 64*mat_size);
    InitMatrix(mat);

    int pattern_id = atoi(argv[1]);
    double rd_wr_ratio = atof(argv[2]);
    int scale = atoi(argv[3]);

    fprintf(stdout, "pattern id : %d rd_wr ratio : %.2f \n", pattern_id, rd_wr_ratio);

    int k;
    StartTimer();
    for(k = 0 ; k < 4; k++)
        AccessMatrix(mat, pattern_id, rd_wr_ratio, scale);
    StopTimer();

    return 0;
}
