#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include "pthread.h"
#include <stddef.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#define BUFSIZE 1024

int main(int argc, char *argv[]){
	
    // The sample type to use
	static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

    pa_simple *s = NULL;
	int ret = 1;
	int error;

    // Create the recording stream 
    	if (!(s = pa_simple_new(NULL, "rec", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
        	fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        	goto finish;
    	}

	int client_sock;
	struct sockaddr_in address_serv;

	printf("\nCreating the socket....\n");
	if((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket error");
		exit(1);
	}
	
	memset(&address_serv, 0, sizeof(address_serv));
	address_serv.sin_family = AF_INET;
   	address_serv.sin_port = htons(8000);
    
    	if(inet_pton(AF_INET, argv[1], &address_serv.sin_addr)<=0){
    		perror("pton error");
    	} 

	printf("\nWaiting for connection....\n");
    	if((connect(client_sock, (struct sockaddr *)&address_serv, sizeof(address_serv))) < 0){
    		perror("failed connection...");
    		exit(1);
    	}
    	printf("\nConnection established....\n\n");

    // Sending recorded data
    	while(1){
		uint8_t buff[BUFSIZE];
		
		/* Record some data ... */
		if (pa_simple_read(s, buff, sizeof(buff), &error) < 0) {
		    fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
		    goto finish;
		}
		
		send(client_sock , buff , sizeof(buff) , 0 ); 
    }
    
    ret = 0;

finish:

    if (s)
        pa_simple_free(s);

    return ret;
}

