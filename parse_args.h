#define DEFAULT_BUFSIZE 4096
#define DEFAULT_PROTOCOL "udp"

struct Args {
    int port;
    int bufsize;
    const char *addr;
    const char *protocol;
    const char *file;   // input or output depending on sender/receiver
};

extern struct Args parse_args(int argc, char *argv[]);
