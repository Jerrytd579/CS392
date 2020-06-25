#/*******************************************************************************
#* Name        : minishell.c
#* Author      : Andrew Chuah, Jerry Cheng
#* Date        : 6/17/2020
#* Description : Creates a minishell within the shell. Shell-ception
#* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
#******************************************************************************/

CC     = gcc
C_FILE = $(wildcard *.c)
TARGET = $(patsubst %.c,%,$(C_FILE))
CFLAGS = -g -Wall -Werror -pedantic-errors

all:
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe
