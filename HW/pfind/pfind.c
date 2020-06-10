/*******************************************************************************
 * Name        : pfind.c
 * Author      : Andrew Chuah, Jerry Cheng
 * Date        : 6/05/2020
 * Description : A program that finds files with a specified set of permissions
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>

static int output = 0;
static char *dir = NULL;
static char *permissions = NULL;
const int formal_perms[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};

void usageMessage(){
    printf("Usage: ./pfind -d <directory> -p <permissions string> [-h]\n");  
}

/**
 * Verifies the validity of the permissions given by the terminal input.
 */
bool verify_perms(const char *permstring){
    if(strlen(permstring) != 9)
        return false;

    for(int i = 0; i < 9;){
        char temp;
        for(int j = 0; j < 3; j++){
            if(j == 0)
                temp = 'r';
            else if(j == 1)
                temp = 'w';
            else
                temp = 'x';
            char cur = *(permstring + i++);
            if(cur != temp && cur != '-')
                return false;
        }
    }
    return true;
}

/**
 * Creates permissions string from directory/file using st_mode bitwise operations to confirm the character.
 */
char *perm_create(const char *file, struct stat *sb){
    char *res = (char*)malloc(sizeof(char) * 10);
    char *cur = res;
    mode_t perm = sb->st_mode;
    for(int i = 0; i < 9;){
        if(perm & *(formal_perms + i++))
            *(cur++) = 'r';
        else
            *(cur++) = '-';
        if(perm & *(formal_perms + i++))
            *(cur++) = 'w';
        else
            *(cur++) = '-';
        if(perm & *(formal_perms + i++))
            *(cur++) = 'x';
        else
            *(cur++) = '-';
    }
    *(cur) = '\0';
    return res;
}

/**
 * Reads the directories recursively, and prints it if the perms are compatible.
 */
void dir_recursive(const char *path){
    DIR *dir;
    struct dirent *entry;
    struct stat sb;

    if((dir = opendir(path)) == NULL){
        fprintf(stderr, "Error: Cannot open directory '%s'. Permission denied.\n", path);
        output++;
        return;
    }
    char name[PATH_MAX];
    name[0] = '\0';
    size_t path_len = 0;
    if(strcmp(path, "/"))
        strncpy(name, path, PATH_MAX);

    path_len = strlen(name) + 1;
    name[path_len - 1] = '/';
    name[path_len] = '\0';
    
    while((entry = readdir(dir)) != NULL){
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        strncpy(name + path_len, entry->d_name, PATH_MAX - path_len);
        if(lstat(name, &sb) < 0){
            fprintf(stderr, "Error: Cannot stat file '%s'. %s.\n", name, strerror(errno));
            continue;
        }
        char *cur = perm_create(name, &sb);
        if(!strcmp(cur, permissions)){
            printf("%s\n", name);
            output++;
        }
        free(cur);
        if(entry->d_type == DT_DIR)
            dir_recursive(name);     
    }
    closedir(dir);
}

int main(const int argc, char * argv[]){
    if(argc == 1){
        usageMessage();
        return EXIT_FAILURE;
    }

    int useopt = 0, dirflag = 0, permsflag = 0;
    while((useopt = getopt(argc, argv, ":d:p:h")) != -1){
        switch (useopt) {
            case 'd':
                dirflag = 1;
                dir = optarg;
                break;
            case 'p':
                permsflag = 1;
                permissions = optarg;
                break;
            case 'h':
                usageMessage();
                return EXIT_SUCCESS;
            case '?':
                fprintf(stderr, "Error: Unknown option '-%c' received.\n", optopt);
                return EXIT_FAILURE;
        }
    }

    if(dirflag != 1){
        printf("Error: Required argument -d <directory> not found.\n");
        return EXIT_FAILURE;
    }

    if(permsflag != 1){
        printf("Error: Required argument -p <permissions string> not found.\n");
        return EXIT_FAILURE;
    }

    char path[PATH_MAX];
    if(realpath(dir, path) == NULL){
        fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", dir, strerror(errno));
        return EXIT_FAILURE;
    }

    if(!verify_perms(permissions)){
        fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", permissions);
        return EXIT_FAILURE;
    }
    dir_recursive(path);

    if(output == 0)
        printf("<no output>\n");

    return EXIT_SUCCESS;
}