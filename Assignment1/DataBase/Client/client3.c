#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <stdlib.h>
  
// structure for message queue 
struct mesg_buffer 
{ 
    long mesg_type; 
    char mesg_text[100]; 
} filename; 
  
int main() 
{ 
    key_t key; 
    int msgid; 
  
    // ftok to generate unique key 
    key = ftok("progfile", 65); 
  
    // msgget creates a message queue and returns identifier
     
    msgid = msgget(key, 0666 | IPC_CREAT); 
    filename.mesg_type = 1;   

    printf("Enter the name of a file you wish to see : "); 
    gets(filename.mesg_text); 
  
    // msgsnd to send message 
    msgsnd(msgid, &filename, sizeof(filename), 0); 
 
   
     //content of file receive
    // msgrcv to receive content 
    msgrcv(msgid, &filename, sizeof(filename), 1, 0); 
  
    // display the content 
    printf("The content of the file is : %s \n",  
                    filename.mesg_text); 
  
    return 0; 
} 
