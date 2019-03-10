#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> 	 
#define PORT 8080 

int main() 
{ 
	struct timeval t1,t2;	
	int sock;
	struct sockaddr_in server;
	char message[5] , server_reply[5];
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("\nCould not create socket");
	}
	printf("Socket created");
	
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 8080 );

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	
	printf("\nconnected\n");
	
	//keep communicating with server
	while(1)
	{
		printf("Enter a character : ");
		scanf("%s" , message);
		gettimeofday(&t1,NULL);

		if( send(sock , message , strlen(message) , 0) < 0)
		{
			printf("\nSend failed");
			return 1;
		}
		
		if(recv(sock , server_reply , 1 , 0) < 0)
		{
			printf("\nreceive failed");
			break;
		}
		gettimeofday(&t2,NULL);
		printf("\nServer reply : %s",server_reply);
		printf("\n time by A:%lu",t2.tv_usec-t1.tv_usec);
	}
	
	close(sock);
	return 0;
}

