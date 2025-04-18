#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>

#include "vdif_lib.h"

// Don't change. This means the configuration is limited but you can still set num_channels, num_threads to > 1
#define NUM_BITS 4
#define DATA_ARRAY_LENGTH 8000   // BYTES
#define SAMPLING_RATE 128000000
#define DURATION 30             // SECONDS


int main(int argc, char *argv[]) {
    if ( argc != 4 ) {
        fprintf(stderr, "Usage: create_vdif  NUM_THREADS  NUM_CHANNELS  FILE\n");
        exit(1);
    }
    for (const char *s=argv[1]; *s!='\0'; ++s)
        if ( !isdigit(*s) ) {
            fprintf(stderr, "Invalid NUM_CHANNELS\n");
            exit(1);
        }
    for (const char *s=argv[2]; *s!='\0'; ++s)
        if ( !isdigit(*s) ) {
            fprintf(stderr, "Invalid NUM_THREADS\n");
            exit(1);
        }

    unsigned num_threads = strtoul(argv[1], NULL, 10);
    unsigned num_channels = strtoul(argv[2], NULL, 10);

    unsigned bits_in_complete_sample = num_channels*NUM_BITS;
    assert(bits_in_complete_sample<=32);
    assert((32/bits_in_complete_sample)*bits_in_complete_sample==32);
    unsigned complete_samples_per_word = 32/bits_in_complete_sample;
    unsigned words_in_frame = DATA_ARRAY_LENGTH/4;
    unsigned samples_per_frame = complete_samples_per_word*words_in_frame;
    printf("Samples per frame %u\n", samples_per_frame);

    assert((SAMPLING_RATE/samples_per_frame)*samples_per_frame==SAMPLING_RATE);
    unsigned frames_per_second = SAMPLING_RATE/samples_per_frame;
    printf("Frames per second %u\n", frames_per_second);
    unsigned total_frames = frames_per_second*num_threads*DURATION;
    printf("Total frames %u\n", total_frames);

    // Most of these stay the same. The ones incremented are in the loop below.
    struct Header h;
    h.validity = 0;
    h.legacy = 0;
    h.seconds_from_ref_epoch = 48;
    h.unassigned = 0;
    h.ref_epoch = 0;
    h.data_frame_number = 0;
    h.version = 0;
    h.num_channels = num_channels;
    h.data_frame_length = HEADER_SIZE+DATA_ARRAY_LENGTH;
    h.data_type = 0;
    h.bits_per_sample = NUM_BITS;
    h.thread_id = 0;
    h.station_id = 0;
    h.extended_data_version = 0;

    unsigned vdif_header[8];
    unsigned char data_array[DATA_ARRAY_LENGTH];    // junk

    int fd;
    if ( (fd=open(argv[3], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1 )
        error(errno, errno, "open: %s", argv[3]);

    for (int second=0; second<DURATION; ++second) {
        for (int frame_index=0; frame_index<frames_per_second; ++frame_index) {
            h.data_frame_number = frame_index;

            for (int thread_index=0; thread_index<num_threads; ++thread_index) {
                h.thread_id = thread_index;
                set_vdif_header(h, vdif_header);
                write(fd, vdif_header, sizeof(vdif_header));
                write(fd, data_array, sizeof(data_array));
            }

        }

        ++h.seconds_from_ref_epoch;
    }
    close(fd);

}


