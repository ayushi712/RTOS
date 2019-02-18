#include <stdio.h> 
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include<string.h>

  
char*data,c;
FILE*fp;

// structure for message queue
struct mesg_buffer 
{ 
    long mesg_type; 
    char mesg_text[100]; 
}filename; 

	void append(char* s, char c)
	{
        	int len = strlen(s);
        	s[len] = c;
        	s[len+1] = '\0';
	}
  
int main() 
{ 
    key_t key; 
    int msgid; 
  
    // ftok to generate unique key 
    key = ftok("progfile", 65); 
  
    // msgget creates a message queue and returns identifier 
    msgid = msgget(key, 0666 | IPC_CREAT); 

    
    while(1)
	{                        //while loop starts
    
    
    // msgrcv to receive filename 
    msgrcv(msgid, &filename, sizeof(filename), 1, 0);
    data = filename.mesg_text; 
      
    msgid = msgget(key, 0666 | IPC_CREAT); 
    filename.mesg_type = 1;
    
    //open file
    fp = fopen(filename.mesg_text, "r");
    
    if (fp==NULL)
    {
     printf("Error while opening the file.\n");
     exit(0);    //exit
    }

    //read contents from file
    c=fgetc(fp);
    while(c!=EOF)
    {
     printf("%c",c);
     append(data, c);
     c=fgetc(fp);
    }

    fclose(fp);
    printf("%s",data);
   

    // msgsnd to send message 
    msgsnd(msgid, &filename, sizeof(filename), 0); 
    
    //fclose(fp);

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
