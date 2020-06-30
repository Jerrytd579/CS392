/*******************************************************************************
* Name        : mtsieve.c
* Author      : Andrew Chuah, Jerry Cheng
* Date        : 6/25/2020
* Description : Multithreading on sieve of Eratosthenes
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <ctype.h>
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
 * Determines whether or not the input string represents a valid integer.
 * A valid integer has an optional minus sign, followed by a series of digits
 * [0-9].
 */
bool is_integer(char *input) {
    int start = 0, len = strlen(input);

    if (len >= 1 && input[0] == '-') {
        if (len < 2)
            return false;
        start = 1;
    }
    for (int i = start; i < len; i++) {
        if (!isdigit(input[i]))
            return false;
    }
    return true;
}

/**
 * Takes as input a string and an in-out parameter value.
 * If the string can be parsed, the integer value is assigned to the value
 * parameter and true is returned.
 * Otherwise, false is returned and the best attempt to modify the value
 * parameter is made.
 */
bool get_integer(char *input, int *value) {
    long long long_long_i;
    if (sscanf(input, "%lld", &long_long_i) != 1)
        return false;
    
    *value = (int)long_long_i;
    if (long_long_i != (long long)*value)
        return false;

    return true;
}

/**
 * Sieve of Eratosthenes implementation
 */
void *sieve(void *ptr){
    int final = 0;

    thread_args *args = (thread_args *)ptr;
    int a = (*args).start;
    int b = (*args).end;
    int n = (int)sqrt(b);

    int low_primes[n-1];
    bool prime[n+1];
    memset(prime, true, sizeof(prime));

    // Standard sieve
    // Taken from GeeksforGeeks
    for(int i = 2; i*i <= n; i++){
        if(prime[i] == true){
            for(int j = i*i; j <= n; j += i)
                prime[j] = false;
        }
    }

    for (int i = 2; i <= n; i++){
        if (prime[i] == true)
            low_primes[i-2] = i;
        else
            low_primes[i-2] = 0;
    }

    bool *high_primes;
    if((high_primes = (bool *)malloc((b-a+2) * sizeof(bool))) == NULL){
        fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    memset(high_primes, true, (b-a+1) * sizeof(bool));
    
    for(int i = 0; i < n-1; i++){
        if(low_primes[i] != 0){
            int p = low_primes[i];
            int x = ceil((double)a/p) * p - a;
            if(a <= p)
                x += p;
            for(int j = x; j <= b-a+1; j += p)
                high_primes[j] = false;
        }
    }

    // Finding digits that are 3
    // Stack Overflow: https://stackoverflow.com/questions/3118490/getting-each-individual-digit-from-a-whole-integer
    for(int i = 0; i < b-a+1; i++){
        if(high_primes[i] == true){
            int digit = 0;
            int primes_true = i + a;

            while(primes_true > 0){
                if((primes_true % 10) == 3)
                    digit++;
                primes_true /= 10;
            }

            if(digit >= 2)
                final++;
        }
    }

    int retval;
    if((retval = pthread_mutex_lock(&lock)) != 0)
        fprintf(stderr, "Warning: Cannot lock mutex. %s.\n", strerror(retval));
    
    total_count += final;

    if((retval = pthread_mutex_unlock(&lock)) != 0)
        fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n", strerror(retval));

    free(high_primes);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if(argc == 1){
        printf("Usage: %s -s <starting value> -e <ending value> -t <num threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int useopt = 0, start = 0, end = 0, num_threads = 0;
    int eflag = 0, sflag = 0, tflag = 0;
    opterr = 0;
    while((useopt = getopt(argc, argv, "s:e:t:")) != -1){
        switch(useopt){
            case 's':
                sflag = 1;
                if(!is_integer(optarg)){
                    fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, useopt);
                    return EXIT_FAILURE;
                }
                else if(!get_integer(optarg, &start)){
                    fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", useopt);
                    return EXIT_FAILURE;
                }
                break;
            case 'e':
                eflag = 1;
                if(!is_integer(optarg)){
                    fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, useopt);
                    return EXIT_FAILURE;
                }
                else if(!get_integer(optarg, &end)){
                    fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", useopt);
                    return EXIT_FAILURE;
                }
                break;
            case 't':
                tflag = 1;
                if(!is_integer(optarg)){
                    fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, useopt);
                    return EXIT_FAILURE;
                }
                else if(!get_integer(optarg, &num_threads)){
                    fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", useopt);
                    return EXIT_FAILURE;
                }
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

    if(optind < argc){
        fprintf(stderr, "Error: Non-option argument '%s' supplied.\n", argv[optind]);
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
        fprintf(stderr, "Error: Number of threads cannot exceed twice the number of processors(%d).\n", num_proc);
        return EXIT_FAILURE;
    }

    printf("Finding all prime numbers between %d and %d.\n", start, end);
    
    int num_values = end - start + 1;
    if(num_threads > num_values)
        num_threads = num_values;

    if (num_threads == 1){
        printf("%d segment:\n", num_threads);
    }
    else if (num_threads > 1){
        printf("%d segments:\n", num_threads);
    }

    int per_segment = num_values / num_threads;
    int remainder = num_values % num_threads;

    int retval;
    if((retval = pthread_mutex_init(&lock, NULL)) != 0){
        fprintf(stderr, "Error: Cannot create mutex. %s.\n", strerror(retval));
        return EXIT_FAILURE;
    }

    pthread_t threads[num_threads];
    thread_args args[num_threads];

    // From Stack Overflow https://stackoverflow.com/questions/16108441/how-can-i-divide-a-range-into-n-equal-bins
    int start2 = start;
    for(int i = 0; i < num_threads; i++){
        args[i].start = start2;
        start2 += (per_segment - 1);
        if(remainder != 0){
            start2++;
            remainder--;
        }
        if(i == num_threads - 1)
            args[i].end = end;
        else
            args[i].end = start2;

        printf("   [%d, %d]\n", args[i].start, args[i].end);
        start2++;

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