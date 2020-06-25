#/*******************************************************************************
#* Name        : main.c
#* Author      : Andrew Chuah, Jerry Cheng
#* Date        : 6/15/2020
#* Description : lab8
#* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
#******************************************************************************/
CC     = gcc
CFLAGS = -g -Wall -Werror -pedantic-errors
OUT_FILE = main

all: main libsum.so
main: main.c
	$(CC) $(CFLAGS) -o $(OUT_FILE) main.c -ldl
libsum.so: sum.c
	$(CC) $(CFLAGS) -shared -fPIC -o libsum.so sum.c
clean:
	rm -f libsum.so $(OUT_FILE) $(OUT_FILE).exe
