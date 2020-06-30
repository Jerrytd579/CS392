#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>
#include "semaphore.h"

#define SCALE_FACTOR 1000000
#define MEM_KEY 1234

int num_running;
int semid;

/**
 * Determines if the string input represent a valid integer.
 */
bool is_integer(const char *input) {
    int start = 0, len = strlen(input), i;

    if (len >= 1 && input[0] == '-') {
        if (len < 2) {
            return false;
        }
        start = 1;
    }
    for (i = start; i < len; i++) {
        if (!isdigit(input[i])) {
            return false;
        }
    }
    return true;
}

/**
 * Attempts to convert the input string into the integer i.
 * Returns true if the conversion was successful, false otherwise.
 */
bool parse_int(const char *input, int *i, const char *usage) {
    long long long_long_i;

    if (strlen(input) == 0) {
        fprintf(stderr,
                "Error: Invalid input '' received for %s.\n", usage);
        return false;
    }
    if (is_integer(input) && sscanf(input, "%lld", &long_long_i) == 1) {
        *i = (int)long_long_i;
        if (long_long_i != *i) {
            fprintf(stderr, "Error: Integer overflow for %s.\n", usage);
            return false;
        }
    } else {
        fprintf(stderr, "Error: Invalid input '%s' received for %s.\n", input,
                usage);
        return false;
    }
    return true;
}

/**
 * Returns a random integer in the range [low, high].
 */
int random_int_in_range(int low, int high) {
    return low + rand() % (high - low + 1);
}

/**
 * Causes the process to sleep for a random amount of time.
 */
void random_sleep() {
    long pid = (long)getpid();
    srand(time(NULL) ^ pid);

    useconds_t sleep_time =
        (useconds_t)random_int_in_range(0 * SCALE_FACTOR, 0.75 * SCALE_FACTOR);

    printf("I, pid %ld, am going to sleep for %.2f seconds.\n", pid,
           (double)sleep_time / SCALE_FACTOR);
    usleep(sleep_time);
    printf("I, pid %ld, have awakened.\n", pid);

    num_running--;
}


int main(int argc, char *argv[]) {
    int retval = EXIT_SUCCESS;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num processes>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int num_processes = 0;
    if (!parse_int(argv[1], &num_processes, "number of processes") ||
            num_processes <= 0) {
        fprintf(stderr, "Error: Invalid number of processes '%s'.\n", argv[1]);
        return EXIT_FAILURE;
    }

    num_running = 0;

    pid_t pid;
    int num_started = 0;
    for (int i = 1; i <= num_processes; i++) {
        if ((pid = fork()) < 0) {
            fprintf(stderr, "Error: process[%d] failed. %s.\n", i,
                    strerror(errno));
            continue;
        } else if (pid == 0) {
            random_sleep();
            exit(EXIT_SUCCESS);  // Make sure child process exits.
        } else {
            num_started++;
            num_running++;
        }
    }

    for (int i = 0; i < num_started; i++) {
        pid_t child_pid = wait(NULL);
        if (child_pid < 0) {
            fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
        } else {
            printf("Child with pid %ld has terminated.\n", (long)child_pid);
        }
    }

    if (num_running == 0) {
        printf("All children are done sleeping.\n");
    } else if (num_running == 1) {
        printf("1 child is still sleeping.\n");
    } else {
        printf("%d children are still sleeping.\n", num_running);
    }

EXIT:
    return retval;
}
