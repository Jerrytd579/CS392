/*******************************************************************************
* Name        : minishell.c
* Author      : Andrew Chuah, Jerry Cheng
* Date        : 6/17/2020
* Description : Creates a minishell within the shell. Shell-ception
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include <wait.h>
#include <setjmp.h>
#include <sys/types.h>
#include <pwd.h>

#define BUFSIZE 8192

// Font colors
#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"
#define RED "\x1b[1;31m"

volatile sig_atomic_t sig_val = 0;
volatile sig_atomic_t child_sig = false;
sigjmp_buf jmpbuf;

/**
 * Signal Handler
 */
void catch_signal(int sig){
    if(!child_sig)
        write(STDOUT_FILENO, "\n", 1);
    
    sig_val = sig;
    siglongjmp(jmpbuf, 1);
}

int main(int argc, char *argv[]){
    char **args;
    int count = 0;

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catch_signal;
    
    if(sigaction(SIGINT, &action, NULL) == -1){
        fprintf(stderr, "%sError: Cannot register signal handler. %s.\n", RED, strerror(errno));
        return EXIT_FAILURE;
    }

    sigsetjmp(jmpbuf, 1);

    while(true){
        if(sig_val == SIGINT){
            if(child_sig){
                for(int i = 0; i < count; i++){
                    free(args[i]);
                }
                free(args);
            }
            child_sig = false;
            sig_val = 0;
        }

        count = 0;
        // Gets working directory
        char buffer[BUFSIZE];
        char input[BUFSIZE];

        if(getcwd(buffer, sizeof(buffer)) == NULL){
            fprintf(stderr, "%sError: Cannot get current working directory. %s.\n", RED, strerror(errno));
            return EXIT_FAILURE;
        }

        printf("%s[%s%s%s]$ ", DEFAULT, BRIGHTBLUE, buffer, DEFAULT);
        fflush(stdout);

        ssize_t bytes_read;
        if((bytes_read = read(STDIN_FILENO, input, sizeof(input)-1)) < 0){
            if(errno == EINTR)
                continue;
            else{
                fprintf(stderr, "%sError: Failed to read from stdin. %s.\n", RED, strerror(errno));
                return EXIT_FAILURE;
            }
        }

        input[bytes_read] = '\0';
        // Just the null-terminating character in input
        if(bytes_read == 1)
            continue;

        if((args = (char **)malloc(sizeof(char *) * 1024)) == NULL){
            fprintf(stderr, "%sError: malloc() failed. %s.\n", RED, strerror(errno));
            return EXIT_FAILURE;
        }

        char *temp = (char *)strtok(input, " ");
        int i = 0;
        while(temp != NULL){
            if((args[i] = (char *)malloc(sizeof(char) * (strlen(temp) + 1))) == NULL){
                fprintf(stderr, "%sError: malloc() failed. %s.\n", RED, strerror(errno));
                return EXIT_FAILURE;
            }

            strcpy(args[i], temp);
            char *c;
            if((c = strchr(args[i], '\n')) != NULL)
                *c = '\0';

            temp = (char *)strtok(NULL, " ");
            i++;
            count++;
        }

        args[i] = NULL;
        int j = 0;
        while(args[j] != NULL){
            if(args[j][0] == '\0'){
                free(args[j]);
                args[j] = NULL;
            }
            j++;
        }

        if(args[0] == NULL){
            for(int i = 0; i < count; i++){
                free(args[i]);
            }
            free(args);
            continue;
        }

        // cd command
        if(((strcmp(args[0], "cd\"\"")) == 0) && (args[1] == NULL)){
            struct passwd *p;
            uid_t user = getuid();

            if((p = getpwuid(user)) == NULL)
                fprintf(stderr, "%sError: Cannot get passwd entry. %s.\n", RED, strerror(errno));

            char *home_dir = p->pw_dir;
            if(chdir(home_dir) == -1)
                fprintf(stderr, "%sError: Cannot change directory to '%s'. %s.\n", RED, args[1], strerror(errno));
        }
        else if((strcmp(args[0], "cd")) == 0){
            if(args[1] == NULL || args[1][0] == '\0' || ((strcmp(args[1], "~") == 0) && (args[2] == NULL || args[2][0] == '\0'))){
                struct passwd *p;
                uid_t user = getuid();

                if((p = getpwuid(user)) == NULL)
                    fprintf(stderr, "%sError: Cannot get passwd entry. %s.\n", RED, strerror(errno));
                
                char *home_dir = p->pw_dir;
                if(chdir(home_dir) == -1)
                    fprintf(stderr, "%sError: Cannot change directory to '%s'. %s.\n", RED, args[1], strerror(errno));
            }
            else{
                bool arg_count = false;
                if((args[2] != NULL) && (args[2][0] != '\0'))
                    arg_count = true;

                if(!arg_count){
                    if(strncmp(args[1], "~/", 2) == 0){
                        struct passwd *p;
                        uid_t user = getuid();

                        if((p = getpwuid(user)) == NULL)
                            fprintf(stderr, "%sError: Cannot get passwd entry. %s.\n", RED, strerror(errno));

                        char *home_dir = p->pw_dir;
                        char *temp2;
                        temp2 = strstr(args[1], "~/") + 2;
                        strcat(home_dir, "/");
                        strcat(home_dir, temp2);

                        if(chdir(home_dir) == -1)
                            fprintf(stderr, "%sError: Cannot change directory to '%s'. %s.\n", RED, args[1], strerror(errno));
                    }
                    else if(chdir(args[1]) == -1)
                        fprintf(stderr, "%sError: Cannot change directory to '%s'. %s.\n", RED, args[1], strerror(errno));
                }
                else
                    fprintf(stderr, "%sError: Too many arguments to cd.\n", RED);
            }
        }
        // exit command
        else if((strcmp(args[0], "exit")) == 0){
            for(int i = 0; i < count; i++){
                free(args[i]);
            }
            free(args);
            return EXIT_SUCCESS;
        }
        // any other command
        else{
            pid_t pid;

            if((pid = fork()) < 0){
                fprintf(stderr, "%sError: fork() failed. %s.\n", RED, strerror(errno));
                for(int i = 0; i < count; i++){
                    free(args[i]);
                }
                free(args);
                return EXIT_FAILURE;
            }
            else if(pid == 0){
                if(execvp(args[0], args) < 0){
                    fprintf(stderr, "%sError: exec() failed. %s.\n", RED, strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
            else{
                child_sig = true;
                int status;
                if(waitpid(pid, &status, 0) < 0){
                    if(errno == EINTR)
                        continue;
                    else{
                        fprintf(stderr, "%sError: wait() failed. %s.\n", RED, strerror(errno));
                        for(int i = 0; i < count; i++){
                            free(args[i]);
                        }
                        free(args);
                    }
                }
                child_sig = false;
            }
        }
        // Freeing memory
        for(int i = 0; i < count; i++){
            free(args[i]);
        }
        free(args);
        memset(input, 0, BUFSIZE);
    }
    return EXIT_SUCCESS;
}