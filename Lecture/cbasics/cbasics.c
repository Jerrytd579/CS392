#include <stdio.h>

enum months{JAN = 1, FEB, MAR, APL, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC};
enum week{Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday};

int main() {
    unsigned int a = -1;
    printf("%u\n", a);

    char c = (char) 0x41424344;
    printf("%d\n", c);
    return 0;
}