/*******************************************************************************
* Name        : chatclient.c
* Author      : Andrew Chuah, Jerry Cheng
* Date        : 6/30/2020
* Description : Chat client to talk to the server
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"

int client_socket = -1;
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

int handle_stdin(){
    int msglen = get_string(outbuf, MAX_MSG_LEN);
    if(msglen == TOO_LONG)
        fprintf(stderr, "Sorry, limit your message to %d characters.\n", MAX_MSG_LEN);
    else if(msglen == NO_INPUT)
        return EXIT_SUCCESS;
    else if(send(client_socket, outbuf, strlen(outbuf), 0) < 0)
        fprintf(stderr, "Failed to send message to server. %s.\n", strerror(errno));

    if(strcmp("bye", outbuf) == 0){
        printf("Goodbye.\n");
        close(client_socket);
        exit(EXIT_SUCCESS);
    }
    
    return EXIT_SUCCESS;
}

int handle_client_socket(){
    int bytes_recvd = recv(client_socket, inbuf, BUFLEN, 0);
    if(bytes_recvd == -1)
        fprintf(stderr, "Warning: Failed to receive incoming message. %s.\n", strerror(errno));
    if(bytes_recvd == 0){
        fprintf(stderr, "\nConnection to server has been lost.\n");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    inbuf[bytes_recvd] = '\0';
    if(strcmp(inbuf, "bye") == 0)
        printf("\nServer initiated shutdown.\n");
    else
        printf("\n%s\n", inbuf);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]){
    int retval, bytes_recvd;
    fd_set set;

    if(argc != 3){
        fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port;
    if(!parse_int(argv[2], &port, "port number"))
        return EXIT_FAILURE;
    if(port < 1024 || port > 65535){
        fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serv_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    memset(&serv_addr, 0, addrlen);

    int ip_conversion = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
    if (ip_conversion == 0) {
        fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
        return EXIT_FAILURE;
    }
    else if (ip_conversion < 0) {
        fprintf(stderr, "Error: Failed to convert IP address. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    while(true){
        printf("Enter your username: ");
        fflush(stdout);

        int getuser = get_string(username, MAX_NAME_LEN + 1);
        if(getuser == NO_INPUT)
            continue;
        else if(getuser == TOO_LONG)
            fprintf(stderr, "Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
        else
            break;
    }

    printf("Hello, %s. Let's try to connect to the server.\n", username);
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    if(connect(client_socket, (struct sockaddr *)&serv_addr, addrlen) < 0){
        fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN, 0)) == -1) {
        fprintf(stderr, "Error: Failed to receive message from server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    inbuf[bytes_recvd] = '\0';
    if(bytes_recvd == 0){
        fprintf(stderr, "All connections are busy. Try again later.\n");
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    strcpy(outbuf, inbuf);
    printf("\n%s\n\n", outbuf);

    if(send(client_socket, username, strlen(username), 0) < 0){
        fprintf(stderr, "Error: Failed to send username to server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    while(true){
        printf("[%s]: ", username);
        fflush(stdout);

        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        FD_SET(client_socket, &set);

        int select_ret = select(client_socket + 1, &set, NULL, NULL, NULL);

        if(select_ret < 0){
            fprintf(stderr, "Error: select() failed. %s.\n", strerror(errno));
            retval = EXIT_FAILURE;
            goto EXIT;
        }
        else{
            if(FD_ISSET(STDIN_FILENO, &set) != 0)
                handle_stdin();
            if(FD_ISSET(client_socket, &set) != 0)
                handle_client_socket();
        }
    }

EXIT:
    if(fcntl(client_socket, F_GETFD) >= 0)
        close(client_socket);

    return retval;
}