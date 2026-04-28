#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 2
#define ITERATIONS 10000

int shared_variable = 0;
pthread_mutex_t mutex;

void *increment_shared_variable(void *thread_id) {
    long tid;
    tid = (long)thread_id;

    for (int i = 0; i < ITERATIONS; ++i) {
        pthread_mutex_lock(&mutex);
        shared_variable++;
        pthread_mutex_unlock(&mutex);
    }

    printf("Thread %ld finished. shared_variable = %d\n", tid, shared_variable);

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    for (t = 0; t < NUM_THREADS; ++t) {
        printf("Creating thread %ld\n", t);
        rc = pthread_create(&threads[t], NULL, increment_shared_variable, (void *)t);
        if (rc) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    // Join threads
    for (t = 0; t < NUM_THREADS; ++t) {
        rc = pthread_join(threads[t], NULL);
        if (rc) {
            printf("ERROR: return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }

    // Destroy mutex
    pthread_mutex_destroy(&mutex);

    printf("Final shared_variable value: %d\n", shared_variable);

    pthread_exit(NULL);
}
