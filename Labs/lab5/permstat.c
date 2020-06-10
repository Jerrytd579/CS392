/*******************************************************************************
 * Name        : permstat.c
 * Author      : Andrew Chuah, Jerry Cheng
 * Date        : 6/08/2020
 * Description : Lab 5
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
               S_IRGRP, S_IWGRP, S_IXGRP,
               S_IROTH, S_IWOTH, S_IXOTH};

void display_usage(char *progname) {
    printf("Usage: %s <filename>\n", progname);
}

/**
 * TODO
 * Returns a string (pointer to char array) containing the permissions of the
 * file referenced in statbuf.
 * Allocates enough space on the heap to hold the 10 printable characters.
 * The first char is always a - (dash), since all files must be regular files.
 * The remaining 9 characters represent the permissions of user (owner), group,
 * and others: r, w, x, or -.
 */
char* permission_string(struct stat *statbuf) {
    char *res = (char*)malloc(sizeof(char) * 11);
    char *cur = res;
    mode_t perm = statbuf->st_mode;
    *(cur++) = '-';
    for(int i = 0; i < 9;){
        if(perm & *(perms + i++))
            *(cur++) = 'r';
        else
            *(cur++) = '-';
        if(perm & *(perms + i++))
            *(cur++) = 'w';
        else
            *(cur++) = '-';
        if(perm & *(perms + i++))
            *(cur++) = 'x';
        else
            *(cur++) = '-';
    }
    *(cur) = '\0';
    return res;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        display_usage(argv[0]);
        return EXIT_FAILURE;
    }

    struct stat statbuf;
    if (stat(argv[1], &statbuf) < 0) {
        fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", argv[1],
                strerror(errno));
        return EXIT_FAILURE;
    }

    /* TODO
     * If the argument supplied to this program is not a regular file,
     * print out an error message to standard error and return EXIT_FAILURE.
     * For example:
     * $ ./permstat iamdir
     * Error: 'iamdir' is not a regular file.
     */
    if((statbuf.st_mode & S_IFMT) != S_IFREG){
        fprintf(stderr, "Error: '%s' is not a regular file.\n", argv[1]);
        return EXIT_FAILURE;
    }

    char *perms = permission_string(&statbuf);
    printf("Permissions: %s\n", perms);
    free(perms);

    return EXIT_SUCCESS;
}
