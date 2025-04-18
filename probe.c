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
#define FALSE 0

typedef unsigned char byte;

void probe_vdif(int port) {
    struct Header header;
    unsigned int file_header[8];

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in src_addr;
    socklen_t src_len = sizeof(src_addr);


    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(port); // listen on this port
    src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if ( bind(sockfd, (struct sockaddr *)&src_addr, sizeof(src_addr)) == -1 )     // bind to a local address
        error(errno, errno, "bind");

    ssize_t num;


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
            } else counting = TRUE;
            last_sequence_number = sequence_number;
        }

        //printf("write %lu\n", args.sequence_header?num-sizeof(uint64_t):num);
        if ( write(fd, args.sequence_header?buffer+sizeof(uint64_t):buffer, args.sequence_header?num-sizeof(uint64_t):num) == -1 )
            error(errno, errno, "write");
    }


    free(buffer);
    close(fd);
    close(sockfd);
}

int main(int argc, char *argv[]) {
    struct Args args = parse_args(argc, argv, PROBE);

    if ( strcmp(args.protocol, "udp") == 0 ) use_udp(args);
    else if ( strcmp(args.protocol, "tcp") == 0 ) use_tcp(args);
    else {
        fprintf(stderr, "Invalid protocol: %s\n", args.protocol);
        exit(1);
    }

    return 0;
}

