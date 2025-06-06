#include <unistd.h>


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
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#define DATASIZE 128512
char buffer[DATASIZE];


int main(int argc, const char *argv[]) {
	int sock = 0;
	int remote_len = 0;
	int counter = 0;
	struct sockaddr_un remote;
	char send_msg[20];

	if( (sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1  ){
		fprintf(stderr, "Client: Error on socket() call \n");
		return 1;
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, argv[1]);
	remote_len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	printf("Client: Trying to connect... \n");
	if( connect(sock, (struct sockaddr*)&remote, remote_len) == -1 ) {
		fprintf(stderr, "Client: Error on connect call \n");
		return 1;
	}

	printf("Client: Connected \n");

    if( send(sock, send_msg, sizeof(send_msg), 0 ) == -1 ) {
        fprintf(stderr, "Client: Error on send() call \n");
    }

    counter = 0;
	while ( 1 ) {
		ssize_t num;
		if ( (num=recv(sock, buffer, DATASIZE, MSG_WAITALL)) == DATASIZE ) {
			printf("%d ", counter);
			fflush(stdout);
			++counter;
		} else {
			printf("\n%ld bytes received, exiting\n", num);
			break;
		}
	}


	return 0;
}
