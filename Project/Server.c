#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <limits.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <zconf.h>

#define ACK "ACK"
#define NACK "NACK"

int server_sock, client_sock;

typedef struct packet{    
    char data[1024];
}Packet;

typedef struct frame{    
    int frame_kind; //ACK:0, SEQ:1 FIN:2
    int sq_no;
    int ack;
    Packet packet;
}Frame;

void error(char *msg)    
{
    perror(msg);
    exit(0);    
}
// CRC parameters (default values are for CRC-8):
   
const int order = 8;  //CRC poly order
const unsigned long poly = 0x07;  // 'poly' is the CRC poly without leading '1' bit
const int direct = 1; // 'direct' [0,1] specifies the kind of algorithm: 1=direct, no augmented zero bits
const unsigned long crcinit = 0x00; // 'crcinit' is the initial CRC value 
const unsigned long crcxor = 0x00; // 'crcxor' is the final XOR value
const int refin = 0;  // 'refin' [0,1] specifies if a data byte is reflected before processing or not
const int refout = 0; // 'refout' [0,1] specifies if the CRC will be reflected before XOR

// internal global values:
unsigned long crcmask;
unsigned long crchighbit;
unsigned long crcinit_direct;
unsigned long crcinit_nondirect;
unsigned long crctab[256];

unsigned long reflect (unsigned long crc, int bitnum) {     //reflects the crc about the center
    unsigned long i, j=1, crcout=0;
    for (i=(unsigned long)1<<(bitnum-1); i; i= i>>1) {   //here bitnum is order which is 8
        if (crc & i)  //bitwise AND to check whether both bits are one
		crcout=crcout | j;  //bitwise OR
        j= j << 1;  //leftshift by one bit
    }
    return (crcout);
}

void generate_crc_table() {    // generate CRC lookup table 
    int i, j;
    unsigned long bit, crc;
    for (i=0; i<256; i++) {
        crc = (unsigned long)i;
        if (refin) 
		crc=reflect(crc, 8);
        crc = crc << (order-8);

        for (j=0; j<8; j++) {
            bit = crc & crchighbit;
            crc = crc<<1;
            if (bit) 
		crc = crc ^ poly;  //bitwise XOR
        }           
        if (refin) 
		crc = reflect(crc, order);
        crc = crc & crcmask;
        crctab[i]= crc;
    }
}

unsigned long crctablefast (unsigned char* p, unsigned long len) {   // fast lookup table algorithm 
    unsigned long crc = crcinit_direct;
    if (refin) 
	crc = reflect(crc, order);
    if (!refin) 
	while (len--) 
		crc = (crc << 8) ^ crctab[ ((crc >> (order-8)) & 0xff) ^ *p++];
    else 
	while (len--) 
		crc = (crc >> 8) ^ crctab[ (crc & 0xff) ^ *p++];

    if (refout^refin) 
	crc = reflect(crc, order);
    crc = crc ^ crcxor;
    crc = crc & crcmask;

    return(crc);
}

char *string_to_bin(char *s)
{
	if (s == NULL) {
    		return NULL;
  	}
  	size_t slen = strlen(s);
  	errno = 0;
  	char *binary = malloc(slen * CHAR_BIT + 1);
  	if (slen == 0) {
    		*binary = '\0';
    		return binary;
  	}
  	char *ptr;
  	char *start = binary;
  	int i;
  	for (ptr = s; *ptr != '\0'; ptr++) {
    		for (i = CHAR_BIT - 1; i >= 0; i--, binary++) {      //CHAR_BIT:::no of bits in CHAR = 8 bits
     			*binary = (*ptr & 1 << i) ? '1' : '0';
   		}
  	}
  	*binary = '\0';
  	binary = start; 
}

float exponent(float x, int y){
    int res = 1;     // Initialize result
    while (y > 0)
    {
        if (y & 1)
            res = res*x;
        y = y >> 1; // y = y/2
        x = x*x;  // Change x to x^2
    }
    return res;
}

void random_flip(char *rem_message, float BER){    //introduces random errors as per BER into the message and ACK/NACK
    unsigned int i =0;
    srand(time(0));
    for (i=0;i<strlen(rem_message);i++){
            if ((float)rand() / RAND_MAX < BER)
            {
                if (rem_message[i] == '0')
                {
                    rem_message[i] = '1';
                }
                else if (rem_message[i] == '1')
                {
                    rem_message[i] = '0';
                }
                else
                {
                    fprintf(stderr, "Convertion to binary was not successful.\n");
                }
            }
        }
}
 
char *construct_message(char *message,char *temp,int data_len)     // convert the data from bits to char  
{
    unsigned int i;
    unsigned int x=0;
    for (i = 0; i < data_len; i++)
    {
        int j;
        int v = 0;
        for (j = i; j < i + 8; j++)
        {
            if (message[j] == '1')
            {
                v = v + exponent(2, i + 7 - j);
            }
        }
        temp[x++]=(char)v;
        i = i + 7;
    }
    temp[x]='\0';
    return temp;
}
void process(int client_sock,float ber)
{
    int data_len = 1;
    char *temp_ack,*temp_nack,*final_ack,*final_nack;

    do
    {
        char message[1024];
        data_len = read(client_sock, message, 1024);   // receive from the socket
        message[data_len] = '\0';

        char *temp = (char *)malloc(1024*sizeof(char));
        memset(temp,0x00,1024);
        temp = construct_message(message,temp,data_len);
 
        unsigned long msg = crctablefast((unsigned char *)temp, strlen(temp));
        
        memset(temp,0x00,1024);
        temp = construct_message(message,temp,data_len-8);
        
        if (data_len && msg == 0)   // if crc is zeros, it implies that the received msg has no error
        { 
            printf("Message received by server:  %s\n",temp);
            printf("Sending ACK to sender.");
       
            temp_ack =(char *)malloc(64*sizeof(char));
            memset(temp_ack,0x00,64);
            temp_ack = string_to_bin(ACK);
            random_flip(temp_ack,ber);
            final_ack=(char *)malloc(64*sizeof(char));
            memset(final_ack,0x00,64); 
            final_ack = construct_message(temp_ack,final_ack,strlen(temp_ack));
      
            send(client_sock, final_ack, strlen(final_ack), 0);
            free(temp_ack);
            free(final_ack);
        }
        else if (data_len)
        {
            printf("Message received by server:  %s\n",temp);
            printf("Message retrieved had some errors, sending NACK to sender.");
            srand(time(0));
            
            temp_nack =(char *)malloc(64*sizeof(char));
            memset(temp_nack,0x00,64);
            temp_nack = string_to_bin(NACK);
            random_flip(temp_nack,ber);
            final_nack=(char *)malloc(64*sizeof(char));
            memset(final_nack,0x00,64); 
            final_nack = construct_message(temp_nack,final_nack,strlen(temp_nack));
  
            send(client_sock, final_nack, strlen(final_nack), 0);
            free(temp_nack);
            free(final_nack);
        }
        else
        {
            close(client_sock);
        }
        free(temp);
    } while (data_len); // continue till we get data from the client
}

int setup_connection(int port)  
{
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        error("Creating socket.");
    }

    printf("Socket created.\n");

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port); 
    server.sin_addr.s_addr = INADDR_ANY;

    if ((bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0))
    {
        error("Binding.");
    }
    printf("Done binding.\n");

    if ((listen(server_sock, 5)) < 0)
    {
        error("Still listening.");
    }
    printf("Listening.\n");
    return 1;
}

void signal_callback(int sigint){
    printf("Signal received. %d\n", sigint);
    close(client_sock);
    close(server_sock);
    exit(sigint);
}

int main(int argc, char **argv){
    if (argc != 2)
    {
        fprintf(stderr, "\nType like this : ./server port\n");
        return EXIT_FAILURE;
    }
    signal(SIGINT, signal_callback);  //signal handler
    float BER;
    printf("Enter BER (probability of bit errors): ");
    scanf("%f", &BER);

    setup_connection(atoi(argv[1]));
    int i;
    unsigned long bit, crc;

    crcmask = ((((unsigned long)1<<(order-1))-1)<<1)|1;
    crchighbit = (unsigned long)1<<(order-1);

    generate_crc_table();

    crcinit_direct = crcinit;
    crc = crcinit;
    for (i=0; i < order; i++) {

        bit = crc & 1;
        if (bit) 
		crc = crc ^ poly;
        crc = crc >> 1;
        if (bit)
		crc = crc | crchighbit;
    }
    crcinit_nondirect = crc;

    while (true)
    {
        struct sockaddr_in client;
        unsigned int len;
        int pid;

        if ((client_sock = accept(server_sock, (struct sockaddr *)&client, &len)) < 0)
        {
            error("Accepting connection.");
        }
        printf("Accepted.\n");
        pid = fork();

        if (pid < 0)
        {
            error ("forking.");
        }

        if (pid == 0)
        {
            close(server_sock);
	    process(client_sock,BER);
            printf("Client served.Server can be terminated now!\n");
            return EXIT_SUCCESS;
        }
        else
        {
            close(client_sock);
        }
    }
}
