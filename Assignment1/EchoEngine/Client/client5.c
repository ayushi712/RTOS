#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h>
#include<time.h>  
 
 
// structure for message queue 
struct mesg_buffer 
{ 
    long mesg_type; 
    char mesg_text[100]; 
} message; 
  

int main() 
{ 
    key_t key; 
    int msgid;
    clock_t start, end;
    double cpu_time_used; 
  
    // ftok to generate unique key 
    key = ftok("progfile", 65); 
  
    
    // msgget creates a message queue and returns identifier 
    msgid = msgget(key, 0666 | IPC_CREAT); 
    message.mesg_type = 1; 
  
    printf("Write any character : "); 
    gets(message.mesg_text); 
  
    // msgsnd to send message 
    msgsnd(msgid, &message, sizeof(message), 0); 

    //clock for timer 
    start = clock();
  
    // display the message 
    printf("Data send is : %s \n", message.mesg_text); 
    
    

    //message receive
    // msgrcv to receive message 
    msgrcv(msgid, &message, sizeof(message), 1, 0); 
  
    // display the message 
    printf("Data Received is : %s \n", message.mesg_text); 
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time is: %f\n",cpu_time_used);                  
  
    return 0; 
} 
