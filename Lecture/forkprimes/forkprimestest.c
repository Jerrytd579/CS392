#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Brute force method to test if a number is prime. */
bool is_prime(unsigned long n) {
    for (int div = 2, end = (int)sqrt(n); div <= end; div++) {
        if (n % div == 0) {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seconds>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse the sleep argument.
    int sleep_secs = atoi(argv[1]);
    if (sleep_secs <= 0) {
        fprintf(stderr,
                "Error: Invalid number of seconds '%s' for sleep time.\n",
                argv[1]);
        return EXIT_FAILURE;
    }

    pid_t pid;
    if ((pid = fork()) == 0) {
        // Child process finds and displays primes forever.
        unsigned long num = 2;
        unsigned int primes_count = 0;
        while (true) {
            if (is_prime(num)) {
                primes_count++;
                printf("Prime %u is %lu.\n", primes_count, num);
            }
            num++;
        }
    }

    // Sleep for the specified time and then kill the child process.
    sleep(sleep_secs);
    if (kill(pid, SIGINT) == -1) {
        fprintf(stderr, "Error: kill() failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
