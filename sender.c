#include <arpa/inet.h>
#include <netdb.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parse_args.h"


void use_tcp(struct Args args) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int fd;
    int num;
    void *buffer;

    if ( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1 )
        error(errno, errno, "socket");

    if ( (server=gethostbyname(args.addr)) == NULL ) {
        fprintf(stderr,"No such host: %s\n", args.addr);
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(args.port);
    if ( connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) == -1 )
        error(errno, errno, "connect");

    // send file
    if ( (fd=open(args.file, O_RDONLY)) == -1 )
        error(errno, errno, "open: %s", args.file);

    if ( (buffer=malloc(args.bufsize)) == NULL ) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    while ( (num=read(fd, buffer, sizeof(buffer))) > 0 ) {
        if ( write(sockfd, buffer, num) == -1 )
            error(errno, errno, "write");
    }


    free(buffer);
    close(fd);
    close(sockfd);
}

void use_udp(struct Args args) {
    void *buffer;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(args.port); // destination port
    inet_aton(args.addr, &dest_addr.sin_addr); // destination IP

    // send file
    int fd;
    if ( (fd=open(args.file, O_RDONLY)) == -1 )
        error(errno, errno, "open: %s", args.file);


    int num;
    if ( (buffer=malloc(args.bufsize)) == NULL ) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    while ( (num=read(fd, buffer, sizeof(buffer))) > 0 ) {
        if ( sendto(sockfd, buffer, num, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1 )
            error(errno, errno, "sendto");
    }

    free(buffer);
    close(fd);
    close(sockfd);

}

int main(int argc, char *argv[]) {
    struct Args args = parse_args(argc, argv, SENDER);

    if ( strcmp(args.protocol, "udp") == 0 ) use_udp(args);
    else use_tcp(args);

    return 0;
}
