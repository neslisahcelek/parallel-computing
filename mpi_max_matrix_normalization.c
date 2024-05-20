#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_RESET_ALL  "\x1b[0m"

void generate_matrix(float *matrix, int rows, int cols) {
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (float)rand() / RAND_MAX * 100.0;
    }
}

void print_matrix_tabular(float *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%.2f\t", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

void print_matrix_max_value_highlighted(float *matrix, int rows, int cols) {
    float max_value = -FLT_MAX;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i * cols + j] > max_value) {
                max_value = matrix[i * cols + j];
            }
        }
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i * cols + j] == max_value) {
                printf(ANSI_COLOR_YELLOW "%5.2f " ANSI_RESET_ALL, matrix[i * cols + j]);
            } else {
                printf("%5.2f ", matrix[i * cols + j]);
            }
        }
        printf("\n");
    }
}




int main(int argc, char *argv[]) {
    int rank, size;
    int N;
    float *matrix = NULL;
    float *sub_matrix;
    int rows_per_proc;
    float local_max = -FLT_MAX;
    float global_max;

    if (argc != 2) {
        printf("Usage: %s <matrix_size>\n", argv[0]);
        return -1;
    }

    N = atoi(argv[1]);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    rows_per_proc = N / size;
    sub_matrix = (float *)malloc(rows_per_proc * N * sizeof(float));
 
    if (rank == 0) {
        matrix = (float *)malloc(N * N * sizeof(float));
        generate_matrix(matrix, N, N);
        printf("Original matrix:\n");
        print_matrix_max_value_highlighted(matrix, N, N);
    }

    MPI_Scatter(matrix, rows_per_proc * N, MPI_FLOAT, sub_matrix, rows_per_proc * N, MPI_FLOAT, 0, MPI_COMM_WORLD);

    printf("Process %d received sub-matrix:\n", rank);
    print_matrix_tabular(sub_matrix, rows_per_proc, N); // Use tabular print function

    for (int i = 0; i < rows_per_proc * N; i++) {
        if (sub_matrix[i] > local_max) {
            local_max = sub_matrix[i];
        }
    }

    MPI_Allreduce(&local_max, &global_max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
   
    if (rank == 0) {
        printf("\nGlobal maximum: %.2f\n", global_max);
    }

    for (int i = 0; i < rows_per_proc * N; i++) {
        sub_matrix[i] /= global_max;
    }

    printf("Process %d normalized sub-matrix:\n", rank);
    print_matrix_tabular(sub_matrix, rows_per_proc, N);

    free(sub_matrix);
    if (rank == 0) {
        free(matrix);
    }

    MPI_Finalize();
    return 0;
}
