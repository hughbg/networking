#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "vdif_lib.h"

void check_bits(unsigned int n, unsigned max_bits, const char *s) {
    unsigned int count = 0;
    while (n) {
        ++count;
        n >>= 1;
    }
    if ( count > max_bits ) {
        fprintf(stderr, "%s value %u has more than %u bits (%u)\n", s, n, max_bits, count);
        exit(1);
    }
}

void check_zero(unsigned n, const char *s) {
    if ( n != 0 ) {
        fprintf(stderr, "%s value %u must be zero\n", s, n);
        exit(1);
    }
}



// Read and structure header values
struct Header parse_header(unsigned *header) {
        struct Header h;
        bzero(&h, sizeof(struct Header));

        h.validity = header[0] >> 31;
        h.legacy = (header[0] >> 30) & 1;
        h.seconds_from_ref_epoch = header[0] & 0x3fffffff;
        h.unassigned = header[1] >> 30;
        h.ref_epoch = (header[1] >> 24) & 0x3f;
        h.data_frame_number = header[1] & 0xffffff;
        h.version = header[2] >> 29;
        h.num_channels = 1 << ((header[2] >> 24)  & 0x1f);
        h.data_frame_length = (header[2] & 0xffffff)*8;
        h.data_type = header[3] >> 31;
        h.bits_per_sample = ((header[3] >> 26) & 0x1f) + 1;
        h.thread_id = (header[3] >> 16) & 0x3ff;
        h.station_id = header[3] & 0xffff;
        h.extended_data_version = header[4] >> 24;
        return h;
}

void set_vdif_header(struct Header h, unsigned *header) {
        unsigned shifts, x;

        check_zero(h.validity, "validity");
        check_zero(h.legacy, "legacy");
        check_bits(h.seconds_from_ref_epoch, 30, "seconds_from_ref_epoch");
        check_zero(h.unassigned, "unassigned");
        check_bits(h.ref_epoch, 6, "ref_epoch");
        check_bits(h.data_frame_number, 24, "data_frame_number");
        check_zero(h.version, "version");
        // check num channels below
        check_bits(h.data_frame_length, 24, "data_frame_length");
        if ( h.data_frame_length%8 != 0 ) {
                fprintf(stderr, "data_frame_length must be a multiple of 8\n");
                exit(1);
        }
        check_zero(h.data_type, "data_type");
        check_bits(h.bits_per_sample-1, 5, "bits_per_sample-1");
        check_bits(h.thread_id, 10, "thread_d");
        check_bits(h.station_id, 16, "station_id");
        check_zero(h.extended_data_version, "extended_data_version");

        bzero(header, HEADER_SIZE);

        header[0] = h.seconds_from_ref_epoch;
        header[1] = (h.ref_epoch << 24) | h.data_frame_number;

        if ( h.num_channels == 0 ){
                fprintf(stderr, "Number of channels is 0\n");
                exit(1);
        }

        x = h.num_channels;
        shifts = 0;

        while ( (x&1) == 0 ) {
                x >>= 1;
                ++shifts;
        }
        if ( shifts > 0x1f ) {
                fprintf(stderr, "log2(Number of channels) is more than 5 bits\n");
                exit(1);
        }

        // test chan is power of 2
        if ( x != 1 ) {
                fprintf(stderr, "Number of channels is not power of 2\n");
                exit(1);
        }
        header[2] = shifts << 24;
        header[2] |= h.data_frame_length/8;

        header[3] = ((h.bits_per_sample-1) << 26) | (h.thread_id << 16) | h.station_id;
}


void print_header(struct Header h) {
        printf("valid %u leg %u s_from_ref_epoch %u unass %u ref_epoch %u frame_num %u vers %u num_ch %u frame_len %u data_t %u num_bits %u thr_id %u stat_id %u edv %u\n",
               h.validity, h.legacy, h.seconds_from_ref_epoch, h.unassigned, h.ref_epoch, h.data_frame_number, h.version, h.num_channels, h.data_frame_length, h.data_type, h.bits_per_sample, h.thread_id, h.station_id, h.extended_data_version);
}

/*
int main() {
        struct Header h, h1;
        unsigned header[8];


        bzero(&h, sizeof(struct Header));

        h.seconds_from_ref_epoch = 48;
        h.ref_epoch = 63;
        h.data_frame_number = 9876;
        h.num_channels = 1024;
        h.data_frame_length = 7784;
        h.bits_per_sample = 7;
        h.thread_id = 459;
        h.station_id = 6789;

        set_vdif_header(h, header);

        h1 = parse_header(header);
        print_header(h1);

}*/
