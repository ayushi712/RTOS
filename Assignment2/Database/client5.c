#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/time.h>
//#include <arpa/inet.h> 
//#include <sys/types.h>
#define PORT 8080  

/*int main() 
{ 
	//struct timeval t1,t2;	
	
	struct sockaddr_in server;
	char message[100] , server_reply[100];
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("\nCould not create socket");
	}*/

int main() 
{ 
    //struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char msg[100] , msg_from_server[100];
    clock_t start, end;
    double cpu_time_used; 
    //char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
	
	printf("\nSocket created");
	
	//memset(&serv_addr, '0', sizeof(serv_addr));
	
	//server.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( 8080 );

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
	printf("\nAddress supported\n");
	
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
	printf("\nconnected\n");
	

	while(1)
	{
		printf("\nEnter the name of file : ");
		scanf("%s" , msg);
		//gettimeofday(&t1,NULL);
		//clock for timer 
    		start = clock();
		
		if( send(sock , msg , sizeof(msg) , 0) < 0)
		{
			printf("\nSend failed");
			return -1;
		}
		memset(msg_from_server,0,sizeof(msg_from_server));
		
		if(recv(sock , msg_from_server , 1024 , 0) < 0)
		{
			printf("\nnot received");
			break;
		}
		//gettimeofday(&t2,NULL);
		printf("\nmsg from server : %s",msg_from_server);
		memset(msg_from_server,0,sizeof(msg_from_server));
		//printf("\ntime by A:%lu",t2.tv_usec-t1.tv_usec);
		end = clock();
    		cpu_time_used = ((double) (end - start)) / SO_PEERSEC;
//CLOCKS_PER_SEC;
    		printf("Time is: %f\n",cpu_time_used); 

	}
	
	close(sock);
	return 0;
}
