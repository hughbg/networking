#include <stdint.h>
#define DEFAULT_BUFSIZE 4096
#define DEFAULT_PROTOCOL "UDP"
#define SENDER 8204
#define RECEIVER 6505

struct Args {
    uint64_t sequence_header;
    int port;
    size_t bufsize;
    const char *addr;
    const char *protocol;   // TCP or UDP so has to be 3-character string
    const char *file;   // input or output depending on sender/receiver
};

extern struct Args parse_args(int argc, char *argv[], int);
