
// Interesting things from the VDIF header
struct Header {
        unsigned seconds_from_ref_epoch;
        unsigned ref_epoch;
        unsigned data_frame_number;
        unsigned num_channels;
        unsigned data_frame_length;
        unsigned bits_per_sample;
        unsigned thread_id;
};

// Read and structure header values
extern struct Header parse_header(unsigned *header);

// Print header in known text format that can be piped into other tools
extern void print_header(struct Header h);
