#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h>
#include<time.h> 
#define MAX_SIZE 100
  
// structure for message queue
char str[MAX_SIZE];
int i;
char * data;


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
  

    //clock for timer 
    start = clock();
    
    while(1)
	{                        //while loop starts

	// msgrcv to receive message
    	msgrcv(msgid, &message, sizeof(message), 1, 0);
    	data = message.mesg_text; 
     
    
  
        // display the message 
        printf("Data Received is : %s \n",data); 
  
        msgid = msgget(key, 0666 | IPC_CREAT); 
        message.mesg_type = 1; 
    
   
        //algorithm to convert
        for(i=0; message.mesg_text[i]!='\0'; i++)
    		{                //for loop starts
        	
        	if(message.mesg_text[i]>=97 && message.mesg_text[i]<=122)
        		{
           		message.mesg_text[i] = message.mesg_text[i] - 32;
        		}
		else if(message.mesg_text[i]>=65 && message.mesg_text[i]<=90)
			{
			message.mesg_text[i] = message.mesg_text[i] + 32;
			}
   		}                //for loop ends

 
        
        printf("Converted string is: %s \n",message.mesg_text); 
        end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Time is: %f\n",cpu_time_used); 
  
        //msgsnd to send message 
    	msgsnd(msgid, &message, sizeof(message), 0);
	
        printf("Press c and enter to continue, ~ and enter to quit" );
        if(getchar()=='~')
		{
	  	break;	
		}
	 
	
 	}                        //while loop ends
 
	
    // to destroy the message queue 
    msgctl(msgid, IPC_RMID, NULL); 
  
    return 0; 
}
