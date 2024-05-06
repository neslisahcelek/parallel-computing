#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(int argc, char** argv) {
    int N = atoi(argv[1]);
    int process_number, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &process_number);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (N % process_number != 0) {
        if (rank == 0) {
            fprintf(stderr, "The number of elements must be divisible by the number of processors.\n");
        }
        MPI_Finalize();
        return 1;
    }

    int n_per_process = N / process_number;
    int *global_data = NULL;
    int *local_data = malloc(n_per_process * sizeof(int));

    // Initialize the array 
    if (rank == 0) {
        global_data = malloc(N * sizeof(int));
        for (int i = 0; i < N; i++) {
            global_data[i] = i + 1;  // Fill the array 
        }
    }

    // Scatter the array to all processors
    MPI_Scatter(global_data, n_per_process, MPI_INT, local_data, n_per_process, MPI_INT, 0, MPI_COMM_WORLD);

    // Find the maximum value in each process
    int local_max = INT_MIN;
    for (int i = 0; i < n_per_process; i++) {
        if (local_data[i] > local_max) {
            local_max = local_data[i];
        }
    }

    // Print local maximum value and process rank
    printf("Process %d: Local maximum value is %d\n", rank, local_max);

    int global_max;
    // Get the global maximum 
    MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("The maximum value is %d\n", global_max);
    }

    // Cleanup
    free(local_data);
    if (rank == 0) {
        free(global_data);
    }

    MPI_Finalize();
    return 0;
}
