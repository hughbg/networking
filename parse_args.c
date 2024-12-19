#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "parse_args.h"


void usage(enum Program which) {
    fprintf(stderr,"Usage: %s [ -h ] [ -b buffer_size ] [ -p TCP|UDP ] %sport %s\n", which==SENDER?"sender":"receiver",
            which==SENDER?"to_host ":"", which==SENDER?"input_file ":"output_file");
    if ( which == SENDER )
        fprintf(stderr, "-h: Prepend a sequence number to each packet (type uint64). Ignored for TCP. Default: FALSE\n");
    else
        fprintf(stderr, "-h: Expect a sequence number prepended to each packet (type uint64). Cannot be used with TCP. Default: FALSE\n");
    fprintf(stderr, "-b: Size of network packet (excluding sequence number). Default: %d\n", DEFAULT_BUFSIZE);
    fprintf(stderr, "-p: Network protocol, TCP or UDP. Default: %s\n", DEFAULT_PROTOCOL);
    if ( which == RECEIVER ) {
        fprintf(stderr, "-k: VDIF STREAM ONLY. Peek at the input stream and report the first VDIF header. Do not write to file but exit. Default: FALSE\n");
        fprintf(stderr, "    VDIF frames must be sent individually in each network packet, so that the header is at the front of every packet.\n");
    }
    exit(1);
}

const char *lower_str(const char *old_str) {
    int len = strlen(old_str);
    char *new_str = malloc(len+1);
    for (int i=0; i<len+1; ++i)
        new_str[i] = tolower(old_str[i]);

    return new_str;
}

struct Args parse_args(int argc, char *argv[], enum Program which) {
    int c;
    struct Args args;

    args.sequence_header = args.peek = false;
    args.protocol = lower_str(DEFAULT_PROTOCOL);   // internally just operate with lower case version
    args.bufsize = DEFAULT_BUFSIZE;
    args.addr = "none";

    opterr = 0;

    while ((c = getopt(argc, argv, "hkb:p:")) != -1)
        switch (c) {
        case 'h':
            args.sequence_header = true;
            break;
        case 'k':
            if ( which == RECEIVER )
                args.peek = true;
            else {
                fprintf(stderr, "Unknown option -k.\n");
                exit(1);
            }
            break;
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
            if ( strcmp(optarg, "tcp") == 0 || strcmp(optarg, "udp") == 0 || strcmp(optarg, "TCP") == 0 || strcmp(optarg, "UDP") ) {
                args.protocol = lower_str(optarg);
            } else {
                    fprintf(stderr, "Invalid protocol: %s\n", optarg);
                    exit(1);
            }

            break;
        case '?':
            if (optopt == 'b' || optopt == 'p' )
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else
                fprintf(stderr, "Unknown option -%c.\n", optopt);
            exit(1);
        default:
            exit(1);
        }

    if ( args.sequence_header && strcmp(args.protocol, "tcp") == 0 ) {
        fprintf(stderr, "sequence numbering (-h) cannot be used with TCP\n");
        exit(1);
    }

    if ( which == SENDER ) {
        // sender, should only be addr, port and filename left
        if ( argc-optind != 3 ) usage(which);

        args.addr = argv[optind];

        // check port
        for (const char *s=argv[optind+1]; *s!='\0'; ++s)
            if ( !isdigit(*s) ) {
                fprintf(stderr, "Invalid port\n");
                exit(1);
            }


        args.port = atoi(argv[optind+1]);
        args.file = argv[optind+2];

    } else if ( which == RECEIVER ) {
        // receiver, should only be port and filename left

        if ( argc-optind != 2 ) usage(which);

        for (const char *s=argv[optind]; *s!='\0'; ++s)
            if ( !isdigit(*s) ) {
                fprintf(stderr, "Invalid port\n");
                exit(1);
            }

        args.port = atoi(argv[optind]);
        args.file = argv[optind+1];

    } else {
        fprintf(stderr, "Unknown program for parse_args\n");
        exit(1);

    }

    printf("sequence header %s bufsize %lu protocol %s addr %s port %d file %s\n", args.sequence_header?"TRUE":"FALSE", args.bufsize, args.protocol, args.addr, args.port, args.file);
    return args;
}
