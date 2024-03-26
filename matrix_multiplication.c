#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct timeval start, end;
double dif;

// optimal matrix multiplication algorithm
void cache_friendly_multiply(double *a, double *b, double *c, int n)
{
    int block_size = 64;
    for (int i = 0; i < n; i += block_size)
    {
        for (int j = 0; j < n; j += block_size)
        {
            for (int k = 0; k < n; k += block_size)
            {
                // Loop within blocks for better cache utilization
                for (int ii = i; ii < i + block_size && ii < n; ++ii)
                {
                    for (int jj = j; jj < j + block_size && jj < n; ++jj)
                    {
                        double sum = 0.0;
                        for (int kk = k; kk < k + block_size && kk < n; ++kk)
                        {
                            sum += a[ii * n + kk] * b[kk * n + jj];
                        }
                        c[ii * n + jj] = sum;
                    }
                }
            }
        }
    }
}

// standard matrix multiplication algorithm
void standard_multiply(double *a, double *b, double *c, int n)
{
    int i, j, k;

    for (i = 0; i < n; ++i)
    {
        for (j = 0; j < n; ++j)
        {
            for (k = 0; k < n; ++k)
            {
                c[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
    }
}

// compare standard and optimal matrix multiplication algorithms
int main()
{
    int arr[5] = {512, 1024, 2048, 4096, 8192};

    for (int i = 0; i < 5; ++i)
    {
        int n = arr[i];

        double *a = (double *)malloc(n * n * sizeof(double));
        double *b = (double *)malloc(n * n * sizeof(double));
        double *c = (double *)malloc(n * n * sizeof(double));

        // Initialize matrices
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                a[i * n + j] = i * n + j;
                b[i * n + j] = i * n + j;
                c[i * n + j] = 0.0;
            }
        }

        printf("Matrix size: %d x %d\n", n, n);

        gettimeofday(&start, NULL);

        cache_friendly_multiply(a, b, c, n);

        gettimeofday(&end, NULL);
        dif = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
        printf("Elapsed Time (Cache friendly): %3.3f\n", dif);

        // Reset c for standard multiplication
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                c[i * n + j] = 0.0;
            }
        }

        gettimeofday(&start, NULL);

        standard_multiply(a, b, c, n);

        gettimeofday(&end, NULL);
        dif = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
        printf("Elapsed Time (Standard): %3.3f\n", dif);

        free(a);
        free(b);
        free(c);
    }

    return 0;
}
