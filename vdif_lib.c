#include "stdio.h"
#include "vdif_lib.h"

// Read and structure header values
struct Header parse_header(unsigned *header) {
        struct Header h;
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

void print_header(struct Header h) {
        printf("valid %u leg %u s_from_ref_epoch %u unass %u ref_epoch %u frame_num %u vers %u num_ch %u frame_len %u data_t %u num_bits %u thr_id %u stat_id %u edv %u\n",
               h.validity, h.legacy, h.seconds_from_ref_epoch, h.unassigned, h.ref_epoch, h.data_frame_number, h.version, h.num_channels, h.data_frame_length, h.data_type, h.bits_per_sample, h.thread_id, h.station_id, h.extended_data_version);
}
