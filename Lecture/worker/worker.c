#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Volatile variables are always read from memory and not from a register.
   Compiler optimizations are disabled for volatile variables.
   The type sig_atomic_t is an integer that can be accessed as an atomic entity
   even in the presence of asynchronous interrupts.
 */
volatile sig_atomic_t signal_val = 0;

/* Signal handler -- keep it simple */
void catch_signal(int sig) {
    signal_val = sig;
}

/**
 * Main function.
 * Run the program in the background: ./worker &
 * Try running these commands from another terminal:
 * kill -INT <pid of worker>
 * kill -TERM <pid of worker>
 * And to really terminate the process:
 * kill -9 <pid of worker>
 */
int main() {
    struct sigaction action;

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catch_signal;
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(SIGINT, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (sigaction(SIGTERM, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    while (true) {
        if (!signal_val) {
            printf("I am busy working. Do not disturb me!\n");
        } else if (signal_val == SIGINT) {
            printf("ARGH! I told you not to disturb me!\n");
            signal_val = 0;
        } else if (signal_val == SIGTERM) {
            printf("YOU TRIED TO TERMINATE ME?\n");
            signal_val = 0;
        }
        sleep(1);
    }
    return EXIT_SUCCESS;
}
