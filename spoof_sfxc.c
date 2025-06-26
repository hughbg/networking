/*
 * What the jive5ab socket code does is as follows. This is the standard server side sequence of calls.
 * In threadfns.cc:sfxcwriter()
 *
 * unlink the existing socket file
 * create an AF_UNIX SOCK_STREAM socket
 * set the socket mode to blocking and SO_REUSEADDR      !non-standard?
 * setup const struct sockaddr
 * call bind
 * call listen
 * call accept
 *     when accepted.....
 * close original socket that listening on
 * read 20 characters
 * loop sending data blocks
 *
 * What sfxc has to do is as follows:
 *
 * create AF_UNIX socket
 * call connect
 * write 20 characters
 * loop receiving data blocks
 *
 * The buffer size of the data here is fixed and discovered by using mem2sfxc=bits_per_sample : 4;
 * I don't what affects the buffer size.
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char *basename (const char *filename) {
  char *p = strrchr (filename, '/');
  return p ? p + 1 : (char *) filename;
}


#define FALSE 0
#define TRUE 1

#define DATA_PACK_SIZE 128512   // bytes
#define FRAMES_PER_DATA_PACK 16
#define PAYLOAD_SIZE 8000    // bytes
#define HEADER_SIZE 32   // bytes
#define FRAMES_PER_SECOND 8000


unsigned char buffer[DATA_PACK_SIZE];


int valid_all(unsigned char *buff) {
	unsigned int count = 0;
	
	for (int i=0; i<FRAMES_PER_DATA_PACK; ++i) {
		// buff[3] to get the top byte of first header word
		count += buff[3] >> 7;
		//print_header(h);
		buff += HEADER_SIZE+PAYLOAD_SIZE;
	}
	
	if ( ! (count == 0 || count == FRAMES_PER_DATA_PACK) ) {
		fprintf(stderr, "Data pack has a mixture of valid/invalid frames, %d valid\n", count);
		sleep(10);
		exit(1);
	}

	return count == 0;
}


const char *vdif_output(const char *unix_socket) {
	static char vdif_name[FILENAME_MAX];
	const char* socket = basename(unix_socket);
	sprintf(vdif_name, "sfxc_%s.vdif", socket);
	return vdif_name;
}

int main(int argc, const char *argv[]) {
	// 2 steps to handshake: 1. connect to socket, 2. write a short message
	int sock = 0;
	int remote_len = 0;
	int counter = 0;
	struct sockaddr_un remote;
	char send_msg[20];

	if ( argc != 2 ) {
		fprintf(stderr, "Usage: spoof_sfxc unix_socket\n"); sleep(5);
		return 1;
	}

	if( (sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1  ){
		fprintf(stderr, "spoof_sfxc: Error on socket() call \n"); sleep(5);
		return 1;
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, argv[1]);
	remote_len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if( connect(sock, (struct sockaddr*)&remote, remote_len) == -1 ) {
		fprintf(stderr, "spoof_sfxc: Error on connect call \n"); sleep(5);
		return 1;
	}

	printf("spoof_sfxc: connected \n");

	// This short message is the handshake with jive5ab
    if( send(sock, send_msg, sizeof(send_msg), 0 ) == -1 ) {
        fprintf(stderr, "spoof_sfxc: Error on send() call \n"); sleep(5);
		return 1;
    }
    
    // Going to write the output to a file.
    int file_fd; const char *fname = vdif_output(argv[1]);
	if ( (file_fd=open(fname, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1 ) {
		error(errno, errno, "open: %s", fname); sleep(5);
		return 1;
	}
    


    counter = 0;
	int last_is_valid = FALSE;
	while ( 1 ) {
		ssize_t num;
		if ( (num=recv(sock, buffer, DATA_PACK_SIZE, MSG_WAITALL)) == DATA_PACK_SIZE ) {

			int is_valid = valid_all(buffer);
			

			if ( counter == 0 ) {
				printf(is_valid?".":"x");
				counter += FRAMES_PER_DATA_PACK;
			} else {
				if ( is_valid != last_is_valid ) {
					printf("%.2fs\n", (float)counter/FRAMES_PER_SECOND);
					printf(is_valid?".":"x");
					counter = FRAMES_PER_DATA_PACK;

				} else {
					printf(is_valid?".":"x");
					counter += FRAMES_PER_DATA_PACK;
					if ( counter%FRAMES_PER_SECOND == 0 ) printf("%us", counter/FRAMES_PER_SECOND);
				}
			}
							
			fflush(stdout); 
			
			last_is_valid = is_valid;
			
			if ( write(file_fd, buffer, DATA_PACK_SIZE) == -1 )
				error(errno, errno, "write");

			
		} else {
			printf("\n%ld bytes received, exiting\n", num);
			break;
		}

	}

	close(sock);
	close(file_fd);
	return 0;
}
