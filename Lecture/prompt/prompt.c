#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 128

sigjmp_buf jmpbuf;

void catch_signal(int sig) {
    write(STDOUT_FILENO, "\n", 1);
    /* Jump back to min, don't return from the function.
       Give 1 back to sigsetjmp to distinguish it from the first time
       sigsetjmp returned. */
    siglongjmp(jmpbuf, 1);
}

int main() {
    struct sigaction action;

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catch_signal;
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(SIGINT, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    char buf[BUFSIZE];
    /* Saves information about the calling environment, so you can return to
       this place later, sigsetjmp returns 0 the first time. When it returns
       from siglongjmp, it returns the value supplied in siglongjump. */
    sigsetjmp(jmpbuf, 1);
    do {
        printf("What is your name? ");
        fflush(stdout);

        ssize_t bytes_read = read(STDIN_FILENO, buf, BUFSIZE - 1);
        if (bytes_read > 0) {
            buf[bytes_read - 1] = '\0';
        }
        if (bytes_read == 1) {
            continue;
        }
        if (strncmp(buf, "exit", 5) == 0) {
            printf("Bye.\n");
            break;
        }
        printf("Hi, %s!\n", buf);
    } while (true);
    return EXIT_SUCCESS;
}
