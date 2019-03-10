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
    int server_fd;
    int length;
    int new_socket;
    int opt = 1;
    int clients[5]; 
    int perform;
    int i ;
    int valread;
    int sock; 
    int max;  
    struct sockaddr_in address;  
    char*msg;
    char reply[100] = "\t Hello User \n";	
    char buffer[100];
	
    fd_set readfds; 
	
    for (i=0; i<5; i++) 
    { 
	clients[i]=0; 
    } 
	
    if((server_fd=socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
	perror("socket failed"); 
	exit(EXIT_FAILURE); 
    } 
	
    if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt))<0) 
    { 
	perror("setsockopt"); 
	exit(EXIT_FAILURE); 
    } 
	
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT ); 
		
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
    { 
	perror("bind failed"); 
	exit(EXIT_FAILURE); 
    }  
	
    if (listen(server_fd, 5) < 0) 
    { 
	perror("listen FAILED"); 
	exit(1); 
    } 
		 
    length = sizeof(address); 
 
    while(1) 
    { 
	FD_ZERO(&readfds); 
	FD_SET(server_fd, &readfds); 
	max = server_fd; 
	 
	for ( i = 0 ; i < 5 ; i++) 
	{ 
		sock = clients[i]; 
		if(sock > 0) 
			FD_SET( sock , &readfds); 
			
		if(sock > max) 
			max = sock; 
	} 
	
	perform = select( max + 1 , &readfds , NULL , NULL , NULL); 
	
	if ((perform < 0) && (errno!=EINTR)) 
	{ 
		printf("error selecting"); 
	} 
	
	if (FD_ISSET(server_fd, &readfds)) 
	{ 
		if ((new_socket=accept(server_fd,(struct sockaddr*)&address, (socklen_t*)&length))<0) 
		{ 
			perror("error accepting"); 
			exit(1); 
		} 
			
		printf("New connection , socket fd is %d,\nIP is: %s,\nport:%d \n",new_socket,inet_ntoa(address.sin_addr), ntohs (address.sin_port)); 
		
		if( send(new_socket, msg, strlen(msg), 0) != strlen(msg) ) 
		{ 
			perror("error sending"); 
			exit(1);
		} 
			
		for (i = 0; i < 5; i++) 
		{ 
			if( clients[i] == 0 ) 
			{ 
				clients[i] = new_socket; 	
				break; 
			} 
		} 
	} 
		
	for (i = 0; i < 5; i++) 
	{ 
		sock = clients[i]; 
				
		if (FD_ISSET(sock , &readfds)) 
		{ 
			if ((valread = read(sock,buffer,1024)) == 0) 
			{ 	
				getpeername(sock, (struct sockaddr*)&address ,(socklen_t*)&length); 
				close(sock); 
				clients[i] = 0; 
			} 
			else
			{ 
                              i=0;
				while(buffer[i]!='\0')
				{
					if(buffer[i]>='a' && buffer[i]<='z')
							buffer[i]=buffer[i]-32;
					else if(buffer[i]>='A' && buffer[i] <='Z')
							buffer[i]=buffer[i]+32;
				i=i+1;		
				}
						
				//buffer[valread] = '\0'; 
				send(sock , buffer , strlen(buffer) , 0 ); 
					
			} 
		} 
	} 
} 
	
return 0; 
} 
