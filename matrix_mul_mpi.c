#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Function to read a matrix from a binary file
void read_matrix(const char* filename, int* size, int** matrix) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    fread(size, sizeof(int), 1, file);
    *matrix = (int*)malloc((*size) * (*size) * sizeof(int));
    fread(*matrix, sizeof(int), (*size) * (*size), file);
    
    fclose(file);
}

// Function to write a matrix to a binary file
void write_matrix(const char* filename, int size, int* matrix) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    fwrite(&size, sizeof(int), 1, file);
    fwrite(matrix, sizeof(int), size * size, file);
    
    fclose(file);
}

// Function to print a matrix to the console
void print_matrix(int size, int* matrix) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%5d ", matrix[i * size + j]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check if the correct number of arguments are provided
    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: %s <matrix A binary file> <matrix B binary file>\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    const char* file_A = argv[1];
    const char* file_B = argv[2];

    int matrix_size;
    int *A = NULL, *B = NULL;

    // Read matrices A and B from files in process 0
    if (rank == 0) {
        read_matrix(file_A, &matrix_size, &A);
        read_matrix(file_B, &matrix_size, &B);
    }

    // Broadcast the matrix size to all processes
    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for matrices A and B in all processes
    if (rank != 0) {
        A = (int*)malloc(matrix_size * matrix_size * sizeof(int));
        B = (int*)malloc(matrix_size * matrix_size * sizeof(int));
    }

    // Broadcast matrices A and B to all processes
    MPI_Bcast(A, matrix_size * matrix_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, matrix_size * matrix_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate the chunk sizes and displacements for each process
    int chunk_size = matrix_size / size;
    int remainder = matrix_size % size;
    int *sendcounts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    int *recvcounts = (int*)malloc(size * sizeof(int));
    int *recvdispls = (int*)malloc(size * sizeof(int));

    int sum = 0;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (chunk_size + (i < remainder ? 1 : 0)) * matrix_size;
        displs[i] = sum;
        sum += sendcounts[i];
    }

    sum = 0;
    for (int i = 0; i < size; i++) {
        recvcounts[i] = sendcounts[i] / matrix_size * matrix_size;
        recvdispls[i] = sum;
        sum += recvcounts[i];
    }

    // Allocate memory for local chunks of matrix A
    int *local_A = (int*)malloc(sendcounts[rank] * sizeof(int));
    MPI_Scatterv(A, sendcounts, displs, MPI_INT, local_A, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for the local result matrix
    int *local_C = (int*)calloc(sendcounts[rank], sizeof(int));

    double start_time = MPI_Wtime();

    // Perform local matrix multiplication
    for (int i = 0; i < sendcounts[rank] / matrix_size; i++) {
        for (int j = 0; j < matrix_size; j++) {
            for (int k = 0; k < matrix_size; k++) {
                local_C[i * matrix_size + j] += local_A[i * matrix_size + k] * B[k * matrix_size + j];
            }
        }
    }

    int *C = NULL;
    if (rank == 0) {
        C = (int*)malloc(matrix_size * matrix_size * sizeof(int));
    }

    // Gather the local result matrices into the final matrix C
    MPI_Gatherv(local_C, recvcounts[rank], MPI_INT, C, recvcounts, recvdispls, MPI_INT, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    // Print the time taken for the matrix multiplication
    if (rank == 0) {
        printf("Time taken for matrix multiplication: %f seconds\n", end_time - start_time);
        write_matrix("C.bin", matrix_size, C);
        free(C);
    }

    free(local_A);
    free(local_C);
    if (rank == 0) {
        free(A);
        free(B);
    }

    free(sendcounts);
    free(displs);
    free(recvcounts);
    free(recvdispls);

    MPI_Finalize();
    return 0;
}
