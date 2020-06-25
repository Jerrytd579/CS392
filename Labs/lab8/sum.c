/*******************************************************************************
* Name        : sum.c
* Author      : Andrew Chuah, Jerry Cheng
* Date        : 6/15/2020
* Description : lab8
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include "sum.h"

/**
 * TODO:
 * Takes in an array of integers and its length.
 * Returns the sum of integers in the array.
 */
int sum_array(int *array, const int length) {
    int ans = 0;
    for(int i = 0; i < length; i++){
        ans += *(array + i);
    }
    return ans;
}
