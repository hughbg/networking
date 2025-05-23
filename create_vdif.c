#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>

#include "vdif_lib.h"

#define TRUE 1
#define FALSE 0
#define BYTE unsigned char

// Don't change. This means the configuration is limited but you can still set num_channels, num_threads to > 1
#define NUM_BITS 4
#define DATA_ARRAY_LENGTH 8000   // BYTES
#define SEQUENCE_NUMBER_TYPE unsigned long long
#define SAMPLING_RATE 128000000


struct Args {
    unsigned num_channels;       // -c
    unsigned num_threads;        // -t
    unsigned duration;           // -d
    unsigned ref_epoch;         // -r
    unsigned seconds_from_ref_epoch;    //-s
    const char *host, *file;
    unsigned port;
    unsigned sleep;
};

void check_unsigned(const char *s, const char *message) {
    while ( *s != '\0' ) {
        if ( !isdigit(*s) ) {
            fprintf(stderr, "%s\n", message);
            exit(1);
        }
        ++s;
    }

}

void usage() {
    fprintf(stderr, "Usage: create_vdif [ -c num_channels ] [ -t num_threads ] [ -d duration ] [ -u sleep] [ -r ref_epoch ] [ -s seconds_from_ref_epoch ] [ file ] [ host port ]\n");
    fprintf(stderr, "One of [ file ] or [ host port ] must be specified. if host and port are given then the frames are sent via VTP. Otherwise written to the file.\n");
    fprintf(stderr, "\nDefaults:\n");
    fprintf(stderr, "num_channels: 1\n");
    fprintf(stderr, "num_threads: 1\n");
    fprintf(stderr, "duration (seconds): 30\n");
    fprintf(stderr, "sleep (between frames, usec): approximated from calculated frame rate\n");
    fprintf(stderr, "ref_epoch: 0\n");
    fprintf(stderr, "seconds_from_ref_epoch: 0\n");
    
    exit(1);
}


struct Args parse_args(int argc, char *argv[]) {
    int c;
    struct Args args;

    bzero(&args, sizeof(args));    // sets numbers to 0 and strings to NULL
    args.num_channels = 1;
    args.num_threads = 1;
    args.duration = 30;

    opterr = 0;

    while ((c = getopt(argc, argv, "c:t:d:r:s:u:")) != -1)
        switch (c) {
        case 'c':
            // check num_channels
            check_unsigned(optarg, "Invalid number of channels");
            args.num_channels = atoi(optarg);
            break;
        case 't':
            // check num_threads
            check_unsigned(optarg, "Invalid number of threads");
            args.num_threads = atoi(optarg);
            break;
        case 'd' :
            // check duration
            check_unsigned(optarg, "Invalid duration");
            args.duration = atoi(optarg);
            if ( args.duration == 0 ) {
                fprintf(stderr, "duration cannot be 0\n");
                exit(1);
            }
            break;
        case 'u':
            // check sleep
            check_unsigned(optarg, "Invalid sleep usec");
            args.sleep = atoi(optarg);
            break;
        case 'r':
            // check ref_epoch
            check_unsigned(optarg, "Invalid ref_epoch");
            args.ref_epoch = atoi(optarg);
            break;
        case 's':
            // check seconds from ref_epoch
            check_unsigned(optarg, "Invalid seconds_from_ref_epoch");
            args.seconds_from_ref_epoch = atoi(optarg);
           break;

        case '?':
            if (optopt == 'd' || optopt == 'r' || optopt == 's' )
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else
                fprintf(stderr, "Unknown option -%c.\n", optopt);
            exit(1);
        default:
            fprintf(stderr, "Unknown option.\n");
            exit(1);
        }

    if ( argc-optind == 1 ) {    // output is file
        args.file = argv[optind];
        printf("file %s num_channels %u num_threads %u duration %u ref_epoch %u seconds_from_ref_epoch %u\n",
               args.file, args.num_channels, args.num_threads, args.duration, args.ref_epoch, args.seconds_from_ref_epoch);


    } else if ( argc-optind == 2 ) {        // output is stream, have host and port
        args.host = argv[optind];
        check_unsigned(argv[optind+1], "Invalid port");
        args.port = atoi(argv[optind+1]);

        if ( args.port < 1024 ) {
            fprintf(stderr, "Port must be > 1023\n");
            exit(1);
        }

    } else {
        usage();

    }

    return args;
}



int main(int argc, char *argv[]) {
    struct Args args = parse_args(argc, argv);
    int to_file = TRUE;
    struct timespec timer;

    unsigned bits_in_complete_sample = args.num_channels*NUM_BITS;
    assert(bits_in_complete_sample<=32);
    assert((32/bits_in_complete_sample)*bits_in_complete_sample==32);
    unsigned complete_samples_per_word = 32/bits_in_complete_sample;
    unsigned words_in_frame = DATA_ARRAY_LENGTH/4;
    unsigned samples_per_frame = complete_samples_per_word*words_in_frame;
    printf("Samples per frame %u\n", samples_per_frame);

    assert((SAMPLING_RATE/samples_per_frame)*samples_per_frame==SAMPLING_RATE);
    unsigned frames_per_second = SAMPLING_RATE/samples_per_frame;
    printf("Frames per second %u\n", frames_per_second);
    float frame_time = 1/(float)frames_per_second;
    printf("Frame time %.2f usec\n", frame_time*1000000);
    unsigned total_frames = frames_per_second*args.num_threads*args.duration;
    printf("Total frames %u\n", total_frames);

    int fd;
    struct sockaddr_in dest_addr;
    if ( args.file != NULL ) {     // write to file, so open it

        if ( (fd=open(args.file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1 )
            error(errno, errno, "open: %s", args.file);
    
    } else if ( args.host != NULL ) {         // write to network
        fd = socket(AF_INET, SOCK_DGRAM, 0);

        
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(args.port); // destination port
        inet_aton(args.host, &dest_addr.sin_addr); // destination IP
        
        to_file = FALSE;

        timer.tv_sec = 0;
        if ( args.sleep == 0 ) timer.tv_nsec = (int)(frame_time*1000000000*2/3);    // 2/3 is a guess
        else timer.tv_nsec = args.sleep*1000;    // nanosec from usec

    } else {
        fprintf(stderr, "Something has gone badly wrong - unspecified output\n");
        exit(1);
    }

    BYTE *buffer;
    unsigned buf_size = sizeof(SEQUENCE_NUMBER_TYPE)+HEADER_SIZE+DATA_ARRAY_LENGTH;
    if ( (buffer=malloc(buf_size)) == NULL ) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    bzero(buffer, buf_size);
    
    // set pointers to data segments in buffer that will be written. sequence_number is ignored for file output
    SEQUENCE_NUMBER_TYPE *sequence_number = (SEQUENCE_NUMBER_TYPE*)buffer;
    unsigned *vdif_header = (unsigned*)(buffer+sizeof(SEQUENCE_NUMBER_TYPE));
    // the rest of the buffer is the data array, unintitialized
    
    // Create a high-level representation of the header that is easy to manipulate.
    // Most of these stay the same. The ones incremented are in the loop below.
    struct Header h;
    h.validity = 0;
    h.legacy = 0;
    h.seconds_from_ref_epoch = args.seconds_from_ref_epoch;
    h.unassigned = 0;
    h.ref_epoch = args.ref_epoch;
    h.data_frame_number = 0;
    h.version = 0;
    h.num_channels = args.num_channels;
    h.data_frame_length = HEADER_SIZE+DATA_ARRAY_LENGTH;
    h.data_type = 0;
    h.bits_per_sample = NUM_BITS;
    h.thread_id = 0;
    h.station_id = 0;
    h.extended_data_version = 0;
        
    // Now loop through the frames and send the data
    for (int second=0; second<args.duration; ++second) {
        for (int frame_index=0; frame_index<frames_per_second; ++frame_index) {
            h.data_frame_number = frame_index;

            for (int thread_index=0; thread_index<args.num_threads; ++thread_index) {
                h.thread_id = thread_index;
                set_vdif_header(h, vdif_header);
                
                // Actually send the frame
                if ( to_file ) {    // ignore sequence_number
                    write(fd, vdif_header, buf_size-sizeof(SEQUENCE_NUMBER_TYPE));
                } else {
                    if ( sendto(fd, buffer, buf_size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1 )
                        error(errno, errno, "sendto");
                    nanosleep(&timer, NULL);
                    ++(*sequence_number);

                }
                
            }

        }

        ++h.seconds_from_ref_epoch;
    }
    close(fd);

    return 0;

}


