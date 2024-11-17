#include <arpa/inet.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "parse_args.h"

#define TRUE 1

void use_udp(struct Args args) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in src_addr;
    socklen_t src_len = sizeof(src_addr);
    void *buffer;

    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(args.port); // listen on this port
    src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if ( bind(sockfd, (struct sockaddr *)&src_addr, sizeof(src_addr)) == -1 )     // bind to a local address
        error(errno, errno, "bind");

    int fd;
    if ( (fd=open(args.file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1 )
        error(errno, errno, "open: %s", args.file);

    int num;
    if ( (buffer=malloc(args.bufsize)) == NULL ) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    for (;;) {
        num = recvfrom(sockfd, buffer, args.bufsize, 0, (struct sockaddr *)&src_addr, &src_len);
        if ( num == -1 )
            error(errno, errno, "recvfrom");
        if ( num == 0 ) break;
        if ( write(fd, buffer, num) == -1 ) error(errno, errno, "write");
    }


    free(buffer);
    close(fd);
    close(sockfd);
}

void use_tcp(struct Args args) {
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    void *buffer;

    if ( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1 )
       error(errno, errno, "socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(args.port);
    if ( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1 )
        error(errno, errno, "bind");
    if ( listen(sockfd,5) == -1 )
        error(errno, errno, "listen");
    clilen = sizeof(cli_addr);
    if ( (newsockfd=accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1 )
        error(errno, errno, "accept");

    int fd;
    if ( (fd=open(args.file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1 )
        error(errno, errno, "open: %s", args.file);

    if ( (buffer=malloc(args.bufsize)) == NULL ) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    for (;;) {
        if ( (n=read(newsockfd,buffer, sizeof(buffer))) == -1 )
            error(errno, errno, "read");

       if ( n == 0 ) break;
       if ( write(fd, buffer, n) == -1 )
           error(errno, errno, "write");
    }

    free(buffer);
    close(fd);
    close(newsockfd);
    close(sockfd);
}
int main(int argc, char *argv[]) {
    struct Args args = parse_args(argc, argv, RECEIVER);

    if ( strcmp(args.protocol, "udp") == 0 ) use_udp(args);
    else use_tcp(args);

    return 0;
}

