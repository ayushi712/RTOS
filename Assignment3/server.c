#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include "pthread.h"
#include <stddef.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#define buffSIZE 1024
#define length 1024

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

int main(int argc, char const *argv[])
{

    // The sample type to use
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

    pa_simple *s = NULL;
    int ret = 1;
    int error;
    
    if (!(s = pa_simple_new(NULL,argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
	} 
    // Server parameters
    int serv_sock, new_sock;
    struct sockaddr_in address_serv, client_serv;
    int addrlen = sizeof(address_serv);
 
    if((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket  creation Error");
        exit(1);
    }
    
    memset(&address_serv, '\0', sizeof(address_serv));  
    address_serv.sin_family = AF_INET;
    address_serv.sin_addr.s_addr = INADDR_ANY;
    address_serv.sin_port = htons(9090);
    
    printf("\nBinding....\n");  
    if((bind(serv_sock, (struct sockaddr *)&address_serv,sizeof(address_serv))) < 0){
        perror("Bind error");
        exit(1);
    }

    // Listening to clients
    printf("\nlistening....\n");
    if(listen(serv_sock, 2) < 0){
        perror("Listen error");
        exit(1);
    }

    printf("\nWaiting for connection....\n");
    while(1){
        if((new_sock = accept(serv_sock, (struct sockaddr *)&address_serv, (socklen_t*)&addrlen)) >= 0){
            while(1){
                uint8_t buff[buffSIZE];
                ssize_t r;

                int val = read(new_sock, buff, sizeof(buff));
                //printf("Read%d\n", val);
                
                // Playing the recorded message
                if (pa_simple_write(s, buff, sizeof(buff), &error) < 0) {
                    fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
                }
            }
        }    
    }
    ret = 0;
finish:

    if (s)
        pa_simple_free(s);

    return ret;
}
