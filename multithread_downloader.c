#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <pthread.h>
#include <string.h>
#define MAX_THREADS 16
struct download_data {
char *url;
    int thread_id;
    int num_threads;
};

void *download_thread(void *arg) {
    struct download_data *data = (struct download_data *)arg;
 
CURL *curl = curl_easy_init();

curl_easy_setopt(curl, CURLOPT_URL, data->url);
 
    char range[64];
    double chunk_size = (double)(data->num_threads - 1) / data->num_threads;
    snprintf(range, sizeof(range), "%lf-%lf", chunk_size * data->thread_id, chunk_size * (data->thread_id + 1) - 0.00001);
    curl_easy_setopt(curl, CURLOPT_RANGE, range);
 
FILE *fp = fopen("downloaded_file", "ab");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

curl_easy_perform(curl);
 
curl_easy_cleanup(curl);
 
fclose(fp);
 
    return NULL;
}
 
int main(int argc, char **argv) {
    if (argc != 3) {
        return 1;
    }
 
char *url = argv[1];
    int thread_count = atoi(argv[2]);
    if (thread_count > MAX_THREADS || thread_count < 1) {
        printf("Thread count must be between 1 and %d\n", MAX_THREADS);
        return 1;
    }
 
pthread_t threads[thread_count];

for (int i = 0; i < thread_count; i++) {
        struct download_data *data = malloc(sizeof(struct download_data));
        data->url = url;
        data->thread_id = i;
        data->num_threads = thread_count;
 
        pthread_create(&threads[i], NULL, download_thread, data);
    }
 
for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
 
    printf("File downloaded successfully\n");
    return 0;
}

