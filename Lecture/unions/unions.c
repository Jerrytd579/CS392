/*******************************************************************************
 * Name          : unions.c
 * Author        : Brian S. Borowski
 * Version       : 1.0
 * Date          : February 4, 2020
 * Last modified : February 5, 2020
 * Description   : Demonstrates how unions work and shows their bit
 *               : representation in memory.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef union my_union {
    char c;
    short s;
    int i;
} my_union_t;

/**
 * Displays the values of the union's members.
 */
void display(my_union_t u) {
    printf("%c\n", u.c);
    printf("%d\n", u.s);
    printf("%d\n", u.i);
}

/**
 * Displays a line with + symbols at 8-bit markers.
 */
void display_line(unsigned int num_bytes) {
    putchar('+');
    for (int i = num_bytes * 8 - 1; i >= 0; i--) {
        putchar('-');
        if (i % 8 == 0) {
            putchar('+');
        }
    }
    putchar('\n');
}

/**
 * Converts the union into an unsigned int and performs bitwise operations
 * to display the contents of memory occupied by the union.
 */
void display_bits(my_union_t u) {
    unsigned int
        p,
        num_bytes = MIN(sizeof(my_union_t), sizeof(unsigned int));

    memcpy(&p, &u, num_bytes);
    display_line(num_bytes);
    putchar('|');
    for (int i = num_bytes * 8 - 1, mask = 1 << i; i >= 0; i--) {
        if (p & mask) {
            putchar('1');
        } else {
            putchar('0');
        }
        if (i % 8 == 0) {
            putchar('|');
        }
        mask >>= 1;
    }
    putchar('\n');
    display_line(num_bytes);
}

int main() {
    my_union_t u;
    int var0;

    u.i = 0;
    display(u);

    u.c = 'A';
    display(u);

    u.s = 16383;
    display(u);

    var0 = u.c;
    printf("%d\n", u.c); // Why is u.c -1?
    printf("%d\n", var0); // Why is var0 -1?

    display_bits(u);
    return 0;
}
