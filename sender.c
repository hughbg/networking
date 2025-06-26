#include <arpa/inet.h>
#include <netdb.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parse_args.h"

typedef unsigned char byte;


void use_tcp(struct Args args) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int fd;
    int num;
    byte *buffer;

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
    while ( (num=read(fd, buffer, args.bufsize)) > 0 ) {
        if ( write(sockfd, buffer, num) == -1 )
            error(errno, errno, "write");
    }


    free(buffer);
    close(fd);
    close(sockfd);
}

void use_udp(struct Args args) {
    byte *buffer;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in dest_addr;
    struct timespec timer;
    int sequence_number = 0;

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(args.port); // destination port
    inet_aton(args.addr, &dest_addr.sin_addr); // destination IP

    // send file
    int fd;
    if ( (fd=open(args.file, O_RDONLY)) == -1 )
        error(errno, errno, "open: %s", args.file);

    if ( (buffer=(byte*)malloc(args.sequence_header?args.bufsize+sizeof(uint64_t):args.bufsize)) == NULL ) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    timer.tv_sec = 0;
    timer.tv_nsec = args.sleep*1000;    // nanosec from usec

    ssize_t num;
    while ( (num=read(fd, args.sequence_header?buffer+sizeof(uint64_t):buffer, args.bufsize)) > 0 ) {
        if ( args.sequence_header ) {
            *((uint64_t*)buffer) = sequence_number;
            ++sequence_number;
        }
        //printf("sequence %lu sent %lu\n", *((uint64_t*)buffer), num);
        if ( sendto(sockfd, buffer, args.sequence_header?num+sizeof(uint64_t):num, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1 )
            error(errno, errno, "sendto");
        if ( args.sleep > 0 ) nanosleep(&timer, NULL);
    }

    free(buffer);
    close(fd);
    close(sockfd);

}

int main(int argc, char *argv[]) {
    struct Args args = parse_args(argc, argv, SENDER);

    if ( strcmp(args.protocol, "udp") == 0 ) use_udp(args);
    else if ( strcmp(args.protocol, "tcp") == 0 ) use_tcp(args);
    else {
        fprintf(stderr, "Invalid protocol: %s\n", args.protocol);
        exit(1);
    }

    return 0;
}
