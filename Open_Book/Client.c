#include <stdio.h>
#include <stdlib.h>  
#include <sys/types.h>
#include <sys/socket.h>  
#include <string.h> 
#include <unistd.h>
#include <netinet/in.h>  
#include <netdb.h>
#include <arpa/inet.h>  

#define PORT 8080 
#define BUFF_SIZE 10000 
  
int main(int argc, char**argv) 
{  
 struct sockaddr_in addr, cl_addr;  
 int sockfd, ret;
 int temperature,humidity,methane,propane,carbondioxide;  
 char buffer[BUFF_SIZE];
 char * serverAddr;  
 struct hostent * server;
 
 FILE *fptr;
 serverAddr = "127.0.0.1";  
 sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
 if (sockfd < 0) 
 {  
  printf("Error creating socket!\n");  
  exit(1);  
 }  
 
  printf("Socket created...\n");   

 memset(&addr, 0, sizeof(addr));  
 addr.sin_family = AF_INET;  
 addr.sin_addr.s_addr = inet_addr(serverAddr);
 addr.sin_port = PORT;     

 ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));  
 if (ret < 0) 
 {  
  printf("Error connecting to the server!\n");  
  exit(1);  
 }  
  printf("Connected\n");  

 memset(buffer, 0, BUFF_SIZE);
 while(1) 
 {
  strcpy(buffer,"Getdata");
  sleep(2);
  ret = sendto(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *) &addr, sizeof(addr));  
  if (ret < 0) 
  {  
   printf("Error sending data!\n\t-%s", buffer);  
  }

  ret = recvfrom(sockfd, buffer, BUFF_SIZE, 0, NULL, NULL);
  if (ret < 0) 
  {  
   printf("Error receiving data!\n");    
  }
  else 
  {
   fptr = fopen("data.csv","a");
   printf("Data received from server is:%s\n",buffer);
   fprintf(fptr,"%s\n",buffer);
   fclose(fptr);
  }  
 }
 
 return 0;    
}  
