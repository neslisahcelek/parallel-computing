#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

typedef struct {
    int start;  
    int end;    
    double partial_sum; /
} ThreadData;

// Function to compute Pi using Taylor series
void *computePi(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->partial_sum = 0.0;

    // Compute the partial sum for this thread
    for (int i = data->start; i < data->end; i++) {
        if (i % 2 == 0)
            data->partial_sum += 1.0 / (2 * i + 1);
        else
            data->partial_sum -= 1.0 / (2 * i + 1);
    }

    return NULL;
}

int main() {
    int total_ops = 1000000;
    int num_threads;
    struct timeval startTime, endTime;
    double dif;

    printf("Enter the total number of operations: ");
    scanf("%d", &total_ops);

    printf("Enter the number of threads: ");
    scanf("%d", &num_threads);
    num_threads++; // Increment by 1 for the master thread

    // Create array of threads and thread data
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    // Calculate the range for each thread
    int ops_per_thread = total_ops / num_threads;
    int extra_ops = total_ops % num_threads;
    int start = 0;
    int end;

    gettimeofday(&startTime, NULL);

    // Create threads and assigning work to each thread
    for (int i = 0; i < num_threads; i++) {
        end = start + ops_per_thread;
        if (extra_ops > 0) {
            end++;
            extra_ops--;
        }
        thread_data[i].start = start;
        thread_data[i].end = end;

        pthread_create(&threads[i], NULL, computePi, (void *)&thread_data[i]);

        start = end;
    }

    // Collect partial results
    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        pi += thread_data[i].partial_sum;
    }

    // Finalize the value of Pi
    pi *= 4;

    gettimeofday(&endTime, NULL);
    dif = (double)(endTime.tv_sec - startTime.tv_sec) + (double)(endTime.tv_usec - startTime.tv_usec) / 1000000.0;

    double true_pi = 3.14159265358979323846;
    double error_margin = fabs(true_pi - pi);

    // Print results
    printf("Estimated value of Pi: %.15f\n", pi);
    printf("Error Margin: %.15f\n", error_margin);
    printf("Execution Time: %.6f seconds\n", dif);

    return 0;
}
