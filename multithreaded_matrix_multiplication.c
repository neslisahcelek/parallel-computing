#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_SIZE 30000
#define NUM_THREADS 12

struct timeval start, end;
double dif;
int n; // Matrix size
int A[MAX_SIZE][MAX_SIZE], B[MAX_SIZE][MAX_SIZE], C[MAX_SIZE][MAX_SIZE]; 

typedef struct {
    int start_row; 
    int end_row;   
} ThreadData;

// Matrix multiplication with row major order
void *multiply(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    for (int i = data->start_row; i < data->end_row; i++) {
        for (int k = 0; k < n; k++) {
            for (int j = 0; j < n; j++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    
    n = atoi(argv[1]);

     // Initialize matrices with random values
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
            C[i][j] = 0;
        }
    }

    // Create threads
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int chunk_size = n / NUM_THREADS; 

    gettimeofday(&start, NULL);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].start_row = i * chunk_size;
        thread_data[i].end_row = (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, multiply, (void *)&thread_data[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the elapsed time
    gettimeofday(&end, NULL);
    dif = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Elapsed Time %d: %3.3f\n", n, dif);

    return 0;
}
