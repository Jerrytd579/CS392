/*******************************************************************************
* Name        : cpumodel.c
* Author      : Andrew Chuah, Jerry Cheng
* Date        : 6/15/2020
* Description : Displays CPU model
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool starts_with(const char *str, const char *prefix) {
    /* TODO:
       Return true if the string starts with prefix, false otherwise.
       Note that prefix might be longer than the string itself.
     */
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

int main() {
    /* TODO:
       Open "cat /proc/cpuinfo" for reading.
       If it fails, print the string "Error: popen() failed. %s.\n", where
       %s is strerror(errno) and return EXIT_FAILURE.
     */
      FILE *fp;
      if((fp = popen("cat /proc/cpuinfo", "r")) == NULL){
        fprintf(stderr, "Error: popen() failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
      }

    /* TODO:
       Allocate an array of 256 characters on the stack.
       Use fgets to read line by line.
       If the line begins with "model name", print everything that comes after
       ": ".
       For example, with the line:
       model name      : AMD Ryzen 9 3900X 12-Core Processor
       print
       AMD Ryzen 9 3900X 12-Core Processor
       including the new line character.
       After you've printed it once, break the loop.
     */
      char buffer[256];
      while(fgets(buffer, 255, fp)){
        buffer[strlen(buffer) - 1] = '\0';
        if(starts_with(buffer, "model name")){
          for(int i = 1; buffer[i] != '\0'; i++){
            if(buffer[i-1] == ':' && buffer[i] == ' '){
              printf("%s\n", buffer + i + 1);
              break;
            }
          }
          break;
        }
      }

    /* TODO:
       Close the file descriptor and check the status.
       If closing the descriptor fails, print the string
       "Error: pclose() failed. %s.\n", where %s is strerror(errno) and return
       EXIT_FAILURE.
     */
    int status = pclose(fp);
      if(status == -1){
        fprintf(stderr, "Error: pclose() failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
      }

    return !(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS);
}
