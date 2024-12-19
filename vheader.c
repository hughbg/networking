// Program for dumping in text format all headers in a VDIF file. Only VDIF with 32 byte header.
#include <stdio.h>
#include <stdlib.h>
#include "vdif_lib.h"

typedef unsigned char *byte;


int main(int argc, char *argv[]) {
        struct Header header;
        unsigned int file_header[8];
        byte *data = NULL;
        size_t num, data_length;
        FILE *f;

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

                /*
                for (int w=0; w<4; ++w) {
                unsigned word = ((unsigned*)data)[w];

                for (int i=0; i<8; ++i) {
                        printf("%u\n", (word >> i*4) & 0x3);
                }
                }
                return 0;
                */


        }

        fclose(f);

        return 0;

}
