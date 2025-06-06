// Program for dumping in text format all headers in a VDIF file. Only VDIF with 32 byte header.
#include <stdio.h>
#include <stdlib.h>
#include "vdif_lib.h"

typedef unsigned char *byte;

void usage() {
        fprintf(stderr, "Usage: vheader vdif_file\n\n");
        fprintf(stderr, "Print a stream of the headers of the frames in a VDIF file, as text.\n");
        fprintf(stderr, "The format of a header is a sequence of name/value pairs printed on a single line\n");
        fprintf(stderr, "for easy parsing. Example:\n\n");
        fprintf(stderr, "  valid 0 leg 0 s_from_ref_epoch 14067903 unass 0 ref_epoch 48 frame_num 1973 vers 0 num_ch 2 frame_len 8032 data_t 0 num_bits 2 thr_id 0 stat_id 21620 edv 0\n");
        fprintf(stderr, "\nThe names of the fields are shortened version of those given in the VDIF specification.\n");
        exit(1);
}

int main(int argc, char *argv[]) {
        struct Header header;
        unsigned int file_header[8];
        byte *data = NULL;
        size_t num, data_length;
        FILE *f;

        if ( argc != 2 ) usage();

        if ( (f=fopen(argv[1],"rb")) == NULL ) {
                fprintf(stderr, "Falied to open %s\n", argv[1]);
                exit(1);
        }

        while ( 1 ) {

                if ( (num=fread(&file_header, 1, sizeof(file_header), f)) != sizeof(file_header) ) {
                        if ( num == 0 ) break;
                        fprintf(stderr, "Incomplete header\n");
                        return 1;
                }

                header = parse_header(file_header);
                print_header(header);

                data_length = header.data_frame_length-sizeof(file_header);
                if ( data == NULL ) data = malloc(data_length);  // initialize

                if ( (num=fread(data, 1, data_length, f)) != data_length ) {
                        fprintf(stderr, "Incomplete frame\n");
                        return 1;
                }

                
                /*for (int w=0; w<4; ++w) {
                unsigned word = ((unsigned*)data)[w];
                printf("word %u\n", word);
                for (int i=0; i<8; ++i) {
                        printf("%u\n", (word >> i*4) & 0x3);
                }
                }
                return 0;*/
                


        }

        fclose(f);

        return 0;

}
