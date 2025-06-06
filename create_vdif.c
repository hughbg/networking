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
#define SAMPLING_RATE 128000000    // Hz


struct Args {
    unsigned num_channels;       // -c
    unsigned num_threads;        // -t
    unsigned duration;           // -d
    unsigned ref_epoch;         // -r
    unsigned seconds_from_ref_epoch;    //-s
    const char *host, *file;
    unsigned port;
    unsigned switcheroo;
    unsigned sleep;
    int sleep_specified;
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
    fprintf(stderr, "Usage: create_vdif [ -c num_channels ] [ -t num_threads ] [ -d duration ] [ -u sleep] [ -r ref_epoch ]\n\t[ -s seconds_from_ref_epoch ] [ -x switching_time ] [ file ] [ host port ]\n");
    fprintf(stderr, "Create a fake VDIF stream with certain paremeters, and send it to the network, or write it to a file.\n\n");
    fprintf(stderr, "One and only one of [ file ] or [ host port ] must be specified. If host and port are given then the frames are sent via VTP. Otherwise written to the file.\n");
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "-c: channels in VDIF (default: 1)\n");
    fprintf(stderr, "-t: threads in VDIF (default: 1)\n");
    fprintf(stderr, "-d: Time length of VDIF in seconds (default: 30)\n");
    fprintf(stderr, "-u: time to sleep in between sending frames, in usec (default: approximated from calculated frame rate)\n");
    fprintf(stderr, "-r: reference epoch in VDIF (default: 0)\n");
    fprintf(stderr, "-s: seconds from reference epoch in VDIF (default: 0)\n");
    fprintf(stderr, "-x: switching time, see below (default: 0)\n");

    fprintf(stderr, "\nThese values are fixed:\n");
    fprintf(stderr, "Data payload length: %d bytes\n", DATA_ARRAY_LENGTH);
    fprintf(stderr, "Sampling rate: %.2f MHz\n", SAMPLING_RATE/1e6);
    fprintf(stderr, "Bits per sample: %d\n", NUM_BITS);

    fprintf(stderr, "\nThe switching time (X seconds) is implemented so as to implement an intermittent stream.\n");
    fprintf(stderr, "The program will write to the specified output for X seconds, then throw away the output for X seconds,\n");
    fprintf(stderr, "then write to the output for X seconds, and so on. If X is 0, no switching is done.\n");
    
    exit(1);
}


struct Args parse_args(int argc, char *argv[]) {
    int c;
    struct Args args;

    bzero(&args, sizeof(args));    // sets numbers to 0 and strings to NULL and booleans to FALSE
    args.num_channels = 1;
    args.num_threads = 1;
    args.duration = 30;


    opterr = 0;

    while ((c = getopt(argc, argv, "c:t:d:r:s:u:x:")) != -1)
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
            args.sleep_specified = TRUE;
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
        case 'x':
            // check switcheroo time
            check_unsigned(optarg, "Invalid switching time");
            args.switcheroo = atoi(optarg);
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

    if ( args.switcheroo != 0 && args.switcheroo >= args.duration ) {
        fprintf(stderr, "Switching time must be less than the duration\n");
        exit(1);
    }



    if ( argc-optind == 1 ) {    // output is file
        args.file = argv[optind];
        printf("file %s num_channels %u num_threads %u duration %u ref_epoch %u seconds_from_ref_epoch %u switching seconds %u\n",
               args.file, args.num_channels, args.num_threads, args.duration, args.ref_epoch, args.seconds_from_ref_epoch, args.switcheroo);


    } else if ( argc-optind == 2 ) {        // output is stream, have host and port
        args.host = argv[optind];
        check_unsigned(argv[optind+1], "Invalid port");
        args.port = atoi(argv[optind+1]);

        if ( args.port < 1024 ) {
            fprintf(stderr, "Port must be > 1023\n");
            exit(1);
        }
        printf("host %s port %u num_channels %u num_threads %u duration %u ref_epoch %u seconds_from_ref_epoch %u\n",
               args.host, args.port, args.num_channels, args.num_threads, args.duration, args.ref_epoch, args.seconds_from_ref_epoch);

    } else {
        usage();

    }


    return args;
}

int skipping_this_second(unsigned switcheroo, unsigned second) {
    return switcheroo != 0 && (second/switcheroo)%2 == 1;
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
    printf("Theoretical frame time %.2f usec\n", frame_time*1000000);
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
        if ( args.sleep_specified ) timer.tv_nsec = args.sleep*1000;    // nanosec from usec
        else timer.tv_nsec = (int)(frame_time*1000000000*2/3);    // 2/3 is a guess

    } else {
        fprintf(stderr, "Something has gone badly wrong - unspecified output\n");
        exit(1);
    }


    BYTE *vtp_frame;
    unsigned buf_size = sizeof(SEQUENCE_NUMBER_TYPE)+HEADER_SIZE+DATA_ARRAY_LENGTH;
    if ( (vtp_frame=malloc(buf_size)) == NULL ) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    bzero(vtp_frame, buf_size);
    
    // set pointers to data segments in buffer that will be written. sequence_number is ignored for file output
    SEQUENCE_NUMBER_TYPE *sequence_number = (SEQUENCE_NUMBER_TYPE*)vtp_frame;
    unsigned *vdif_frame = (unsigned*)(vtp_frame+sizeof(SEQUENCE_NUMBER_TYPE));
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
    for (unsigned second=0; second<args.duration; ++second) {

        for (int frame_index=0; frame_index<frames_per_second; ++frame_index) {
            h.data_frame_number = frame_index;

            for (int thread_index=0; thread_index<args.num_threads; ++thread_index) {
                h.thread_id = thread_index;
                set_vdif_header(h, vdif_frame);

                // Actually send the frame if not skipping

                if ( !skipping_this_second(args.switcheroo, second) ) {

                    if ( to_file ) {    // ignore sequence_number
                        write(fd, vdif_frame, buf_size-sizeof(SEQUENCE_NUMBER_TYPE));
                    } else {
                        if ( sendto(fd, vtp_frame, buf_size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1 )
                            error(errno, errno, "sendto");
                        if ( args.sleep != 0 ) nanosleep(&timer, NULL);

                    }

                }

                ++(*sequence_number);

            }

        }

        if ( skipping_this_second(args.switcheroo, second) ) {
            //printf("skip second %u\n", second);
            sleep(1);
        }

        ++h.seconds_from_ref_epoch;
    }
    close(fd);


    return 0;

}


