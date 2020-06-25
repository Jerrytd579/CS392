/*******************************************************************************
* Name        : mtsieve.c
* Author      : Andrew Chuah, Jerry Cheng
* Date        : 6/25/2020
* Description : Multithreading on sieve of Eratosthenes
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

int total_count = 0;
pthread_mutex_t lock;

typedef struct arg_struct {
    int start;
    int end;
} thread_args;

/**
 * Sieve of Eratosthenes implementation
 */
void *sieve(void *ptr){
    int count = 0;
    thread_args *args = (thread_args *)ptr;

    int retval;
    if((retval = pthread_mutex_lock(&lock)) != 0)
        fprintf(stderr, "Warning: Cannot lock mutex. %s.\n", strerror(retval));
    
    // something here to add up total count

    if((retval = pthread_mutex_unlock(&lock)) != 0)
        fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n", strerror(retval));

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if(argc == 1){
        printf("Usage: %s -s <starting value> -e <ending value> -t <num threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int useopt = 0, start = 0, end = 0, num_threads = 0;
    int eflag = 0, sflag = 0, tflag = 0;
    while((useopt = getopt(argc, argv, ":e:s:t:")) != -1){
        switch(useopt){
            case 'e':
                end = optarg;
                eflag = 1;
                break;
            case 's':
                start = optarg;
                sflag = 1;
                break;
            case 't':
                num_threads = optarg;
                tflag = 1;
                break;
            case '?':
                if(optopt == 'e' || optopt == 's' || optopt == 't')
                    fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
                else if(isprint(optopt))
                    fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
                else
                    fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
                return EXIT_FAILURE;
        }
    }

    if(argc > 4){
        fprintf(stderr, "Error: Non-option argument '%s' supplied.\n", argv[4]);
        return EXIT_FAILURE;
    }
    if(sflag != 1){
        fprintf(stderr, "Error: Required argument <starting value> is missing.\n");
        return EXIT_FAILURE;
    }
    if(start < 2){
        fprintf(stderr, "Error: Starting value must be >= 2.\n");
        return EXIT_FAILURE;
    }
    if(eflag != 1){
        fprintf(stderr, "Error: Required argument <ending value> is missing.\n");
        return EXIT_FAILURE;
    }
    if(end < 2){
        fprintf(stderr, "Error: Ending value must be >= 2.\n");
        return EXIT_FAILURE;
    }
    if(end < start){
        fprintf(stderr, "Error: Ending value must be >= starting value.\n");
        return EXIT_FAILURE;
    }
    if(tflag != 1){
        fprintf(stderr, "Error: Required argument <num threads> is missing.\n");
        return EXIT_FAILURE;
    }
    if(num_threads < 1){
        fprintf(stderr, "Error: Number of threads cannot be less than 1.\n");
        return EXIT_FAILURE;
    }

    int num_proc = get_nprocs();
    int max_threads = num_proc * 2;
    if(num_threads > max_threads){
        fprintf(stderr, "Error: Number of threads cannot exceed twice the number of processors(%d).\n");
        return EXIT_FAILURE;
    }

    printf("Finding all prime numbers between %d and %d.\n", start, end);

    int num_values = end - start + 1;
    if(num_threads > num_values)
        num_threads = num_values;

    int per_segment = num_values / num_threads;
    int remainder = num_values % num_threads;

    int retval;
    if((retval = pthread_mutex_init(&lock, NULL)) != 0){
        fprintf(stderr, "Error: Cannot create mutex. %s.\n", strerror(retval));
        return EXIT_FAILURE;
    }

    pthread_t threads[num_threads];
    thread_args args[num_threads];

    for(int i = 0; i < num_threads; i++){
        if((retval = pthread_create(&threads[i], NULL, sieve, (void *)(&args[i]))) != 0){
            fprintf(stderr, "Error: Cannot create thread %d. %s.\n", i+1, strerror(retval));
            return EXIT_FAILURE;
        }
    }

    for(int i = 0; i < num_threads; i++){
        if(pthread_join(threads[i], NULL) != 0)
            fprintf(stderr, "Warning: Thread %d did not join properly.\n", i+1);
    }

    if ((retval = pthread_mutex_destroy(&lock)) != 0)
        fprintf(stderr, "Warning: Cannot destroy mutex. %s.\n", strerror(retval));
    
    printf("Total primes between %d and %d with two or more '3' digits: %d\n", start, end, total_count);
    return EXIT_SUCCESS;
}