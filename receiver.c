#include <arpa/inet.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "parse_args.h"
#include "vdif_lib.h"

typedef unsigned char byte;

void use_udp(struct Args args) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in src_addr;
    socklen_t src_len = sizeof(src_addr);
    uint64_t last_sequence_number;
    bool counting=false;
    byte *buffer;


    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(args.port); // listen on this port
    src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if ( bind(sockfd, (struct sockaddr *)&src_addr, sizeof(src_addr)) == -1 )     // bind to a local address
        error(errno, errno, "bind");

    int fd;
    if ( !args.peek ) {
        if ( (fd=open(args.file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1 )
            error(errno, errno, "open: %s", args.file);
    }

    if ( (buffer=malloc(args.sequence_header?args.bufsize+sizeof(uint64_t):args.bufsize)) == NULL ) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    ssize_t num;
    for (;;) {
        num = recvfrom(sockfd, buffer, args.sequence_header?args.bufsize+sizeof(uint64_t):args.bufsize, 0, (struct sockaddr *)&src_addr, &src_len);
        if ( num == -1 )
            error(errno, errno, "recvfrom");
        if ( num == 0 ) break;
        if ( args.sequence_header )  {
            uint64_t sequence_number = *((uint64_t*)buffer);
            //printf("sequence number %lu num %lu\n", sequence_number, num);
            if ( counting ) {
                if ( sequence_number != last_sequence_number+1 )
                    fprintf(stderr, "sequence error: last was %lu just got %lu\n", last_sequence_number, sequence_number);
            } else counting = true;
            last_sequence_number = sequence_number;
        }

        if ( args.peek ) {
            unsigned *vheader = (unsigned*)(args.sequence_header?buffer+sizeof(uint64_t):buffer);
            struct Header header = parse_header(vheader);
            print_header(header);
            exit(0);
        }

        //printf("write %lu\n", args.sequence_header?num-sizeof(uint64_t):num);
        if ( write(fd, args.sequence_header?buffer+sizeof(uint64_t):buffer, args.sequence_header?num-sizeof(uint64_t):num) == -1 )
            error(errno, errno, "write");
    }


    free(buffer);
    if ( !args.peek ) close(fd);
    close(sockfd);
}

void use_tcp(struct Args args) {
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    byte *buffer;

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
    if ( !args.peek ) {
        if ( (fd=open(args.file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1 )
            error(errno, errno, "open: %s", args.file);
    }

    if ( (buffer=malloc(args.bufsize)) == NULL ) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    for (;;) {
        if ( (n=read(newsockfd, buffer, args.bufsize)) == -1 )
            error(errno, errno, "read");

        if ( n == 0 ) break;

        if ( args.peek ) {
            unsigned *vheader = (unsigned*)buffer;        // can't use sequence numbers with tcp, so no check
            struct Header header = parse_header(vheader);
            print_header(header);
            exit(0);
        }

        if ( write(fd, buffer, n) == -1 )
            error(errno, errno, "write");
    }

    free(buffer);
    if ( !args.peek ) close(fd);
    close(newsockfd);
    close(sockfd);
}

int main(int argc, char *argv[]) {
    struct Args args = parse_args(argc, argv, RECEIVER);

    if ( strcmp(args.protocol, "udp") == 0 ) use_udp(args);
    else if ( strcmp(args.protocol, "tcp") == 0 ) use_tcp(args);
    else {
        fprintf(stderr, "Invalid protocol: %s\n", args.protocol);
        exit(1);
    }

    return 0;
}

