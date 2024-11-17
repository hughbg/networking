#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "parse_args.h"



void usage(const char *prog) {
    fprintf(stderr,"Usage: %s -b buffer_size -p [tcp | udp] destination port file\n", prog);
    fprintf(stderr, "Default buffer_size: %d\n", DEFAULT_BUFSIZE);
    fprintf(stderr, "Default protocol: %s\n", DEFAULT_PROTOCOL);
    exit(1);
}


struct Args parse_args(int argc, char *argv[]) {
    int c;
    struct Args args;

    args.protocol = DEFAULT_PROTOCOL;
    args.bufsize = DEFAULT_BUFSIZE;
    args.addr = "none";

    opterr = 0;

    while ((c = getopt(argc, argv, "b:p:")) != -1)
        switch (c) {
        case 'b':
            // check bufsize
            for (const char *s=optarg; *s!='\0'; ++s)
                if ( !isdigit(*s) ) {
                    fprintf(stderr, "Invalid bufsize\n");
                    exit(1);
                }
            args.bufsize = atoi(optarg);
            break;
        case 'p':
            // check protocol
            if ( strcmp(optarg, "tcp") == 0 || strcmp(optarg, "udp") == 0 )
                args.protocol = optarg;
            else {
                    fprintf(stderr, "Invalid protocol: %s\n", optarg);
                    exit(1);
                }

            break;
        case '?':
            if (optopt == 'b' || optopt == 'p' )
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            exit(1);
        default:
            exit(1);
        }

    if ( argc-optind == 3 ) {
        // Assume sender, should only be addr, port and filename left

        args.addr = argv[optind];

        // check port
        for (const char *s=argv[optind+1]; *s!='\0'; ++s)
            if ( !isdigit(*s) ) {
                fprintf(stderr, "Invalid port\n");
                exit(1);
            }


        args.port = atoi(argv[optind+1]);
        args.file = argv[optind+2];

    } else if ( argc-optind == 2 ) {
            // Assume receiver, should only be port and filename left
        for (const char *s=argv[optind]; *s!='\0'; ++s)
            if ( !isdigit(*s) ) {
                fprintf(stderr, "Invalid port\n");
                exit(1);
            }

        args.port = atoi(argv[optind]);
        args.file = argv[optind+1];

    } else usage(argv[0]);

    //printf("bufsize %d protocol %s addr %s port %d file %s\n", args.bufsize, args.protocol, args.addr, args.port, args.file);
    return args;
}
