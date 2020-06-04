/*******************************************************************************
 * Name        : sort.c
 * Author      : Andrew Chuah, Jerry Cheng
 * Date        : 5/29/2020
 * Description : Uses quicksort to sort a file of either ints, doubles, or
 *               strings.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.h"

#define MAX_STRLEN     64 // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum {
    STRING,
    INT,
    DOUBLE
} elem_t;

/**
 * Reads data from filename into an already allocated 2D array of chars.
 * Exits the entire program if the file cannot be opened.
 */
size_t read_data(char *filename, char **data) {
    // Open the file.
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open '%s'. %s.\n", filename,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Read in the data.
    size_t index = 0;
    char str[MAX_STRLEN + 2];
    char *eoln;
    while (fgets(str, MAX_STRLEN + 2, fp) != NULL) {
        eoln = strchr(str, '\n');
        if (eoln == NULL) {
            str[MAX_STRLEN] = '\0';
        } else {
            *eoln = '\0';
        }
        // Ignore blank lines.
        if (strlen(str) != 0) {
            data[index] = (char *)malloc((MAX_STRLEN + 1) * sizeof(char));
            strcpy(data[index++], str);
        }
    }

    // Close the file before returning from the function.
    fclose(fp);
    return index;
}

void displayUsage(){
    printf( "Usage: ./sort [-i|-d] [filename]\n"
    "   -i: Specifies the file contains ints.\n"
    "   -d: Specifies the file contains doubles.\n"
    "   filename: The file to sort.\n"
    "   No flags defaults to sorting strings.\n");  
}

/**
 * Basic structure of sort.c:
 *
 * Parses args with getopt.
 * Opens input file for reading.
 * Allocates space in a char** for at least MAX_ELEMENTS strings to be stored,
 * where MAX_ELEMENTS is 1024.
 * Reads in the file
 * - For each line, allocates space in each index of the char** to store the
 *   line.
 * Closes the file, after reading in all the lines.
 * Calls quicksort based on type (int, double, string) supplied on the command
 * line.
 * Frees all data.
 * Ensures there are no memory leaks with valgrind. 
 */
int main(int argc, char **argv) {
    int useopt = -1, intflag = 0, doubleflag = 0;

    // Parses args with getopt
    while((useopt = getopt(argc, argv, ":id")) != -1){
        switch (useopt) {
            case 'i':
                intflag = 1;
                break;
            case 'd':
                doubleflag = 1;
                break;
            case '?':
                printf("Error: Unknown option '-%c' received.\n", optopt);
                displayUsage();
                return EXIT_FAILURE;
        }
    }
    // Only ./sort
    if (argc == 1){
        displayUsage();
        return EXIT_FAILURE;
    }
    // Too many flags
    if ((intflag + doubleflag) > 1){
        printf("Error: Too many flags specified.\n");
        return EXIT_FAILURE;
    }
    // No input file
    if(argv[optind] == NULL){
        printf("Error: No input file specified.\n");
        return EXIT_FAILURE;
    }
    // Too many input files
    if(optind < argc){
        if(argc - optind != 1){
            printf("Error: Too many files specified.\n");
            return EXIT_FAILURE;
        }
    }

    char** buffer = malloc(MAX_ELEMENTS);
    size_t len = read_data(argv[optind], buffer);

    // Sorting ints
    if(intflag == 1){
        int *copy = malloc(MAX_ELEMENTS * sizeof(int));
        memset(copy, 0, MAX_ELEMENTS);
        for(int i = 0; i < len; i++){
            *(copy+i) = atoi(buffer[i]);
        }
        quicksort(copy, len, sizeof(int), int_cmp);
        for(int j = 0; j < len; j++){
            printf("%d\n", *(copy+j));
        }
        free(copy);
    }
    // Sorting doubles
    else if(doubleflag == 1){
        double *copy = malloc(MAX_ELEMENTS * sizeof(double));
        memset(copy, 0, MAX_ELEMENTS);
        for(int i = 0; i < len; i++){
            *(copy+i) = atof(buffer[i]);
        }
        quicksort(copy, len, sizeof(double), dbl_cmp);
        for(int j = 0; j < len; j++){
            printf("%lf\n", *(copy+j));
        }
        free(copy);
    }
    // Sorting strings
    else{
        char **copy = malloc(MAX_ELEMENTS * sizeof(char*));
        memset(copy, 0, MAX_ELEMENTS);
        for(int i = 0; i < len; i++){
            char *str_copy = (char*)malloc((MAX_STRLEN + 1) * sizeof(char));
            strcpy(str_copy, buffer[i]);
            *(copy+i) = str_copy;
        }
        quicksort(copy, len, MAX_STRLEN, str_cmp);
        for(int j = 0; j < len; j++){
            printf("%s\n", *(copy+j));
            free(copy[j]);
        }
        free(copy);
    }
    for(int i = 0; i < len; i++){
        free(buffer[i]);
    }
    free(buffer);
    return EXIT_SUCCESS;
}
