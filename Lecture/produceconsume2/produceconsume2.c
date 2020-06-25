#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_EXPRS 10
#define QUEUE_LEN 5
#define MAX_STRLEN 16
#define SCALE_FACTOR 1000000

// Function prototypes
void* consume(void *ptr);
void* produce(void *ptr);
int random_int_in_range(int low, int high, unsigned int *seed);

char *queue[QUEUE_LEN];
pthread_mutex_t mutex        = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;
int num_occupied = 0, read_index = 0, write_index = 0;

int random_int_in_range(int low, int high, unsigned int *seed) {
    return low + rand_r(seed) % (high - low + 1); 
}

void *consume(void *ptr) {
    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self());
    int num_consumed = 0;
    while (num_consumed++ < MAX_EXPRS) {
        // Lock the mutex.
        pthread_mutex_lock(&mutex);

        // pthread_cond_wait must be placed in a while loop.
        // pthread_cond_wait atomically unlocks the mutex and waits for the
        // condition variable to be signaled.
        // Unlocking the mutex and suspending on the condition variable is done
        // atomically.
        while (num_occupied <= 0) {
            pthread_cond_wait(&consumer_cond, &mutex);
        }

        // Consume the mathematical expression.
        int a, b;
        sscanf(queue[read_index], "%d + %d", &a, &b);
        printf("Consumer[%d, %d]: %d + %d = %d\n", *(int *)ptr, num_consumed, a,
               b, a + b);
        fflush(stdout);
        read_index = (read_index + 1) % QUEUE_LEN;
        num_occupied--;

        // Notify the producer that there is an opening in the queue.
        pthread_cond_signal(&producer_cond);

        // Unlock the mutex.
        pthread_mutex_unlock(&mutex);

        usleep(
          (useconds_t)random_int_in_range(
            0 * SCALE_FACTOR, 0.5 * SCALE_FACTOR, &seed));
    }
    pthread_exit(NULL);
}

void* produce(void *ptr) {
    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self());
    int num_produced = 0;
    while (num_produced++ < MAX_EXPRS) {
        int a = random_int_in_range(0, 9, &seed),
            b = random_int_in_range(0, 9, &seed);

        // Lock the mutex.
        pthread_mutex_lock(&mutex);

        while (num_occupied >= QUEUE_LEN) {
            pthread_cond_wait(&producer_cond, &mutex);
        }
 
        // Produce the mathematical expression.
        sprintf(queue[write_index], "%d + %d", a, b);
        printf("Producer[%d, %d]: %s\n", *(int *)ptr, num_produced,
               queue[write_index]);
        fflush(stdout);
        write_index = (write_index + 1) % QUEUE_LEN;
        num_occupied++;

        // Notify the consumer that data is available to process.
        pthread_cond_signal(&consumer_cond);

        // Unlock the mutex.
        pthread_mutex_unlock(&mutex);

        usleep(
          (useconds_t)random_int_in_range(
            0 * SCALE_FACTOR, 0.5 * SCALE_FACTOR, &seed));
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[4];
    int thread_ids[4] = {1, 2, 1, 2};

    for (int i = 0; i < QUEUE_LEN; i++) {
        queue[i] = (char *)malloc(MAX_STRLEN * sizeof(char));
    }

    // Create one producer and one consumer.
    int retval;
    if ((retval =
            pthread_create(&threads[0], NULL, produce, &thread_ids[0])) != 0) {
        fprintf(stderr, "Error: Cannot create producer thread 1. %s.\n", 
                strerror(retval));
        return EXIT_FAILURE;
    }
    if ((retval =
            pthread_create(&threads[1], NULL, produce, &thread_ids[1])) != 0) {
        fprintf(stderr, "Error: Cannot create producer thread 2. %s.\n", 
                strerror(retval));
        return EXIT_FAILURE;
    }
    if ((retval =
            pthread_create(&threads[2], NULL, consume, &thread_ids[2])) != 0) {
        fprintf(stderr, "Error: Cannot create consumer thread 1. %s.\n", 
                strerror(retval));
        return EXIT_FAILURE;
    }
    if ((retval =
            pthread_create(&threads[3], NULL, consume, &thread_ids[3])) != 0) {
        fprintf(stderr, "Error: Cannot create consumer thread 2. %s.\n", 
                strerror(retval));
        return EXIT_FAILURE;
    }

    // Wait for all threads to finish.
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    return EXIT_SUCCESS;
}
