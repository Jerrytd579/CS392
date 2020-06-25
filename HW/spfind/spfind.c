/*******************************************************************************
* Name        : spfind.c
* Author      : Andrew Chuah, Jerry Cheng
* Date        : 6/10/2020
* Description : pfind, but using exec to call 'sort' to sort our output from pfind.
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <wait.h>

/**
 * Special case for when the help flag is triggered, or no argument was entered.
 */
bool helpflag(const char *usage, const char *help){
    if(strlen(usage) > strlen(help))
        return false;

    for(int i = 0; i < strlen(usage); i++){
        if(usage[i] != help[i])
            return false;
    }
    return true;
}

int main(int argc, char *argv[]){
    int pfind2sort[2], sort2parent[2];

    if(pipe(pfind2sort) == -1){
        fprintf(stderr, "Error: Failed to create a 'pfind2sort' pipe. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if(pipe(sort2parent) == -1){
        fprintf(stderr, "Error: Failed to create a 'sort2parent' pipe. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    pid_t pid[2];
    // pfind process
    if((pid[0] = fork()) < 0)
        return EXIT_FAILURE;
    else if(pid[0] == 0){
        //sort2parent pipe close attempt
        if(close(sort2parent[0]) == -1){
            fprintf(stderr, "Error: Failed to close 'sort2parent' pipe. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        if(close(sort2parent[1]) == -1){
            fprintf(stderr, "Error: Failed to close 'sort2parent' pipe. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        
        // pfind2sort pipe close attempt
        if(close(pfind2sort[0]) == -1){
            fprintf(stderr, "Error: Failed to close 'pfind2sort' pipe. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        if(dup2(pfind2sort[1], STDOUT_FILENO) == -1){
            close(pfind2sort[1]);
            fprintf(stderr, "Error: Failed to duplicate 'pfind2sort' pipe. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }

        // exec call (execv because we have input that we need to process for pfind (argv))
        if(execv("pfind", argv) < 0){
            fprintf(stderr, "Error: pfind failed.\n");
            return EXIT_FAILURE;
        }
    }

    // sort process
    if((pid[1] = fork()) < 0)
        return EXIT_FAILURE;
    else if(pid[1] == 0){
        if(close(pfind2sort[1]) == -1){
            fprintf(stderr, "Error: Failed to close 'pfind2sort' pipe. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        if(dup2(pfind2sort[0], STDIN_FILENO) == -1){
            close(pfind2sort[0]);
            fprintf(stderr, "Error: Failed to duplicate 'pfind2sort' pipe. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }

        if(close(sort2parent[0]) == -1){
            fprintf(stderr, "Error: Failed to close 'sort2parent' pipe. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        if(dup2(sort2parent[1], STDOUT_FILENO) == -1){
            close(sort2parent[1]);
            fprintf(stderr, "Error: Failed to duplicate 'sort2parent' pipe. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }

        // exec call (execlp because we need to find sort processes matching the supplied argument)
        if(execlp("sort", "sort", NULL) < 0){
            fprintf(stderr, "Error: sort failed.\n");
            return EXIT_FAILURE;
        }
    }

    // parent process
    // closing pfind2sort pipes
    if(close(pfind2sort[0]) == -1){
        fprintf(stderr, "Error: Failed to close 'pfind2sort' pipe. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if(close(pfind2sort[1]) == -1){
        fprintf(stderr, "Error: Failed to close 'pfind2sort' pipe. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // closing sort2parent pipes except the one that takes input
    if(close(sort2parent[1]) == -1){
        fprintf(stderr, "Error: Failed to close 'sort2parent' pipe. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if(dup2(sort2parent[0], STDIN_FILENO) == -1){
        close(sort2parent[0]);
        fprintf(stderr, "Error: Failed to duplicate 'sort2parent' pipe. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // Now we read info we received from process
    char buffer[8192], buffet[8192];
    int line_count = 0;
    ssize_t read_bytes;

    while((read_bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0){
        int i = 0;
        if(read_bytes == -1){
            // If read process was interrupted
            if(EINTR == errno)
                continue;
            else{
                fprintf(stderr, "Error: Failed to read file. %s.\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else if(read_bytes == 0)
            break;
        else{
            while(i < read_bytes){
                if(write(STDOUT_FILENO, &buffer[i], 1) < 0){
                    fprintf(stderr, "Error: Failed to write to terminal. %s.\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if(line_count == 0)
                    buffet[i] = buffer[i];

                if(buffer[i] == '\n')
                    line_count++;

                i++;
            }
        }
    }

    int state;
    bool error = true;
    // Waiting on pid[0] to die
    if(waitpid(pid[0], &state, 0) < 0){
        fprintf(stderr, "Error: waitpid() on pid[0] failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if(!WIFEXITED(state)){
        fprintf(stderr, "WIFEXITED failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if(WEXITSTATUS(state) != EXIT_SUCCESS){
        error = false;
    }

    // Waiting on pid[1] to die
    if(waitpid(pid[1], &state, 0) < 0){
        fprintf(stderr, "Error: waitpid() on pid[1] failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if(!WIFEXITED(state)){
        fprintf(stderr, "WIFEXITED failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if(WEXITSTATUS(state) != EXIT_SUCCESS){
        error = false;
    }

    // Special case if help flag was triggered, makes sure it doesn't print total matches.
    // Also doesn't allow it to print if the processes failed to die.
    if(!helpflag("Usage:", buffet) && error)
        printf("Total matches: %d\n", line_count);
        
    return EXIT_SUCCESS;
}