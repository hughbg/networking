
// Interesting things from the VDIF header
struct Header {
        unsigned validity;
        unsigned legacy;
        unsigned seconds_from_ref_epoch;
        unsigned unassigned;
        unsigned ref_epoch;
        unsigned data_frame_number;
        unsigned version;
        unsigned num_channels;
        unsigned data_frame_length;
        unsigned data_type;
        unsigned bits_per_sample;
        unsigned thread_id;
        unsigned station_id;
        unsigned extended_data_version;
};

// Read and structure header values
extern struct Header parse_header(unsigned *header);

// Print header in known text format that can be piped into other tools
extern void print_header(struct Header h);
