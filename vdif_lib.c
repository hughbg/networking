#include "stdio.h"
#include "vdif_lib.h"

// Read and structure header values
struct Header parse_header(unsigned *header) {
        struct Header h;
        h.seconds_from_ref_epoch = header[0] & 0x3fffffff;
        h.ref_epoch = (header[1] >> 24) & 0x3f;
        h.data_frame_number = header[1] & 0xffffff;
        h.data_frame_length = (header[2] & 0xffffff)*8;
        h.num_channels = 1 << ((header[2] >> 24)  & 0x1f);
        h.bits_per_sample = ((header[3] >> 26) & 0x1f) + 1;
        h.thread_id = (header[3] >> 16) & 0x3ff;
        return h;
}

void print_header(struct Header h) {
        printf("seconds_from_ref_epoch %u ref_epoch %u data_frame_number %u data_frame_length %u num_channels %u bits_per_sample %u thread_id %u\n",
               h.seconds_from_ref_epoch, h.ref_epoch, h.data_frame_number, h.data_frame_length, h.num_channels, h.bits_per_sample, h.thread_id);
}
