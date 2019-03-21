#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define PORT 8080
#define BUFF_SIZE 10000
#define LENGTH 1000     //client address length

void main() 

{
 struct sockaddr_in addr, cl_addr;
 int sockfd, len, ret, newsockfd;
 int temperature,humidity,methane,propane,carbonmonoxide;
 char buffer[BUFF_SIZE],buff[BUFF_SIZE];
 char clientAddr[LENGTH];
 
 time_t s, val = 1;
 struct tm* current_time;
 
 FILE *fptr;
 sockfd = socket(AF_INET, SOCK_STREAM, 0);
 
 if (sockfd < 0) 
 {
  printf("Error creating socket!\n");
  exit(1);
 }
 
  printf("Socket created!\n");

 memset(&addr, 0, sizeof(addr));
 addr.sin_family = AF_INET;
 addr.sin_addr.s_addr = INADDR_ANY;
 addr.sin_port = PORT;
 
 ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
 if (ret < 0) 
 {
  printf("Error binding!\n");
  exit(1);
 }
 printf("Binding done...\n");

 listen(sockfd, 1);

 for (;;) 
 { //infinite loop
  len = sizeof(cl_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &len);
  if (newsockfd < 0) 
  {
    printf("Error accepting connection!\n");
    exit(1);
  }
    printf("Connection accepted...\n");
  
    inet_ntop(AF_INET, &(cl_addr.sin_addr), clientAddr, LENGTH);
   
    for (;;) 
    {
    memset(buffer, 0, BUFF_SIZE);
    s=time(NULL);
    ret = recvfrom(newsockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *) &cl_addr, &len);
    if(ret < 0) 
    {
     printf("Error receiving data!\n");  
     exit(1);
    }
    
     printf("Received data from client is: %s\n",buffer);
  
    if(strcmp(buffer,"Getdata")==0)
    {

      fptr = fopen("sensor1.txt", "r");
      if (fptr == NULL)
      {
       printf("Error opening file.\n");
      }
      else
      {
       fscanf(fptr, "%d", &temperature);
       fclose(fptr);
      }

      
      fptr = fopen("sensor2.txt", "r");
      if (fptr == NULL)
      {
       printf("Error opening file.\n");
      }
      else
      {
       fscanf(fptr, "%d", &humidity);
       fclose(fptr);
      }
      

      fptr = fopen("sensor3.txt", "r");
      if (fptr == NULL)
      {
       printf("Error opening file.\n");
      }      
      else
      {
      fscanf(fptr, "%d", &methane);
      fclose(fptr);
      }
      

      fptr = fopen("sensor4.txt", "r");
      if (fptr == NULL)
      {
       printf("Error opening file.\n");
      }
      else
      {
       fscanf(fptr, "%d", &carbonmonoxide);
       fclose(fptr);
      }

      
      fptr = fopen("sensor5.txt", "r");
      if (fptr == NULL)
      {
       printf("Error opening file.\n");
      }
      else
      {
      fscanf(fptr, "%d", &propane);
      fclose(fptr);
      }
       
       current_time = localtime(&s);
       sprintf(buff,"%02d:%02d:%02d,%d,%d,%d,%d,%d",current_time->tm_hour,current_time->tm_min,current_time->tm_sec,temperature,humidity,methane,carbonmonoxide,propane);
       
       ret = sendto(newsockfd, buff, BUFF_SIZE, 0, (struct sockaddr *) &cl_addr, len);   
       if (ret < 0) {  
       printf("Error sending data!\n");  
       exit(1);     
    }            
    
   }
  }
  close(newsockfd);
 }
}
