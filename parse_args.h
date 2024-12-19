#include <stdint.h>
#include <stdbool.h>
#define DEFAULT_BUFSIZE 4096
#define DEFAULT_PROTOCOL "UDP"

enum Program { SENDER, RECEIVER };

struct Args {
    int port;
    size_t bufsize;
    const char *addr;
    const char *protocol;   // TCP or UDP so has to be 3-character string
    const char *file;   // input or output depending on sender/receiver
    bool peek;          // receiver only
    bool sequence_header; 
};

extern struct Args parse_args(int argc, char *argv[], enum Program);
