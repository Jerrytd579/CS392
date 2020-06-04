/*******************************************************************************
 * Name          : strings.c
 * Author        : Brian S. Borowski
 * Version       : 1.0
 * Date          : February 5, 2020
 * Last modified : February 5, 2020
 * Description   : Demonstrates working with strings as well as taking in
 *               : input via scanf.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Makes a copy of string s via memcpy.
 */
char* my_strdup_mem(const char *s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = strlen(s) + 1;
    char *copy = (char *)malloc(len * sizeof(char));
    if (copy == NULL) {
        return NULL;
    }
    return (char *)memcpy(copy, s, len);
}

/**
 * Makes a copy of string s via pointer arithmetic.
 */
char* my_strdup_ptr(const char *s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = strlen(s) + 1;
    char *copy = (char *)malloc(len * sizeof(char));
    if (copy == NULL) {
        return NULL;
    }
    char *t = copy; // Don't move copy or else you'll lose the pointer to the
                    // beginnning of the char array.
    while (*s) {
        *t++ = *s++;
    }
    *t = '\0';      // Don't forget the null-terminating character.
    return copy;
}

int main() {
    char name[128];
    printf("What is your name? ");
    scanf("%s", name);

    printf("Hello, %s!\n", name);
    printf("Your name has %ld characters.\n", strlen(name));

    char *copy1 = my_strdup_mem(name);
    printf("Hello, %s!\n", copy1);
    printf("Your name has %ld characters.\n", strlen(copy1));

    char *copy2 = my_strdup_ptr(name);
    printf("Hello, %s!\n", copy2);
    printf("Your name has %ld characters.\n", strlen(copy2));

    // Free up the memory you allocated for the duplicates.
    free(copy1);
    free(copy2);

    printf("How old are you? ");
    int age;
    scanf("%d", &age);
    printf("You are %d year(s) old.\n", age);

    printf("How old will you be in 1 year? ");
    scanf("%d", &age);
    printf("You will be %d year(s) old next year.\n", age);

    return 0;
}
