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
#include <sys/time.h>

typedef struct packet {
    char data[1024];
} Packet;

typedef struct frame {
    int frame_kind; //ACK:0, SEQ:1 FIN:2
    int sq_no;
    int ack;
    Packet packet;
} Frame;

void error(char *msg) {
    perror(msg);
    exit(0);
}
struct timeval t1, t2;		//for calculating time

// CRC parameters (default values are for CRC-8):

const int order = 8;
const unsigned long poly = 0x07;
const int direct = 1;
const unsigned long crcinit = 0x00;
const unsigned long crcxor = 0x00;
const int refin = 0;
const int refout = 0;

unsigned long crcmask;
unsigned long crchighbit;
unsigned long crcinit_direct;
unsigned long crcinit_nondirect;
unsigned long crctab[256];  //256 entries for all dividends


unsigned long reflect(unsigned long crc, int bitnum) {   // reflects the bits of 'crc' for reordering
    unsigned long i, j = 1, crcout = 0;

    for (i = (unsigned long) 1 << (bitnum - 1); i; i >>= 1) {
        if (crc & i) crcout |= j;
        j <<= 1;
    }
    return (crcout);
}

void generate_crc_table() {    //generation of CRC lookup table 

    int i, j;
    unsigned long bit, crc;
    for (i=0; i<256; i++) {
        crc = (unsigned long)i;
        if (refin) 
		crc=reflect(crc, 8);
        crc = crc << (order-8);

        for (j=0; j<8; j++) {
            bit = crc & crchighbit;
            crc = crc << 1;
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
		crc = (crc << 8) ^ crctab[((crc >> (order-8)) & 0xff) ^ *p++];
    else 
	while (len--) 
		crc = (crc >> 8) ^ crctab[(crc & 0xff) ^ *p++];

    if (refout^refin) 
	crc = reflect(crc, order);
    crc = crc ^ crcxor;
    crc = crc & crcmask;

    return(crc);
}

char *string_to_bin(char *s) {
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
        // perform bitwise AND for every bit of the character 
        // loop over the input-character bits
        for (i = CHAR_BIT - 1; i >= 0; i--, binary++) {
            *binary = (*ptr & 1 << i) ? '1' : '0';
        }
    }
    *binary = '\0';
    binary = start;   // reset pointer to beginning
    return binary;
}

void random_flip(char *rem_message, float BER) {
    unsigned int i = 0;
    srand(time(0));
    for (i = 0; i < strlen(rem_message); i++) {
        if ((float) rand() / RAND_MAX < BER) {
            if (rem_message[i] == '0') {
                rem_message[i] = '1';
            } else if (rem_message[i] == '1') {
                rem_message[i] = '0';
            } else {
                fprintf(stderr, "Convertion to binary was not successful.\n");
            }
        }
    }
}

int connection_setup(int port, char *addr) {

    struct sockaddr_in serv_addr;
    int client_sock; // client fd
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("Error! failed to open the socket\n");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);  
    serv_addr.sin_addr.s_addr = inet_addr(addr);
    bzero(&serv_addr.sin_zero, 8); // clears the buffer

    struct timeval tv;
    tv.tv_sec = 5;  // 5 Secs timeout
    tv.tv_usec = 0;

    // set socket option for timeout
    setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(struct timeval));

    if (connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in)) < 0) {
        error("connection error.");
    }
    return client_sock;
}

int server_communicate(int client_sock, char *formed_message) {

    int str_len;
    socklen_t adr_sz;
    struct sockaddr_in serv_adr, from_adr;
    Frame ackframe, recv_frame;
    int frame_id;
    int recv_result;

    if (send(client_sock, formed_message, strlen(formed_message), 0) < 0) {
        fprintf(stderr, "Error while sending.\n");
        return -1;
    }
    printf("Message sent. Waiting for ACK/NACK.\n");

    int reply_length;
    char reply[64];

    if ((reply_length = recv(client_sock, reply, 64, 0)) < 0) {
        fprintf(stderr, "Timeout. Re-transmitting.\n");
        return -1;
    }
  
    reply[reply_length] = '\0';
    printf("Reply received: %s\n", reply);
    if (strcmp(reply, "NACK") == 0) {
        fprintf(stderr, "Previous transmission had some error. Re-transmitting.\n");
        return -1;
    } else if (strcmp(reply, "ACK") == 0) {
		gettimeofday(&t2, NULL);				//upto time t2   
        	printf("Total time for acknowledgement: %lu\n",t2.tv_usec - t1.tv_usec);  	//total time taken for succesful reception = t2 - t1
        	return 1;
    } else {
        	fprintf(stderr, "Error in ACK or NACK!! Re-transmitting... \n");
        	return -1;
    }
}

void swap(char* a, char* b){     
    char temp = *a;
    *a = *b;
    *b = temp;
}

void reverse(char str[], int length)    
{ 
    int start = 0; 
    int end = length -1; 
    while (start < end) 
    { 
        swap((str+start), (str+end)); 
        start++; 
        end--; 
    } 
} 

char* itoa(int num, char* str, int base)    
{ 
    int i = 0; 
    bool isNegative = false; 
    if (num == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    }
    if (num < 0 && base == 10) 
    { 
        isNegative = true; 
        num = -num; 
    } 
  
    while (num != 0) 
    { 
        int rem = num % base; 
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; 
        num = num/base; 
    } 
    if (isNegative) 
        str[i++] = '-';   //if negative put a "-" sign
    str[i] = '\0'; 
    reverse(str, i); 
  
    return str; 
} 

int main(int argc, char **argv) {


    if (argc != 3) {
        fprintf(stderr, "Usage ./client ip_addr port");
        return -1;
    }
    float BER;
    printf("Enter BER (probability of bit errors): ");
    scanf("%f", &BER);

    int client_sock;
    if ((client_sock = connection_setup(atoi(argv[2]), argv[1])) == -1) {
        return -1;
    }
    int i;
    unsigned long bit, crc;
    // at first, compute constant bit masks for whole CRC and CRC high bit

    crcmask = ((((unsigned long) 1 << (order - 1)) - 1) << 1) | 1;   //basically calculates negate of (crc & 1)
    crchighbit = (unsigned long) 1 << (order - 1);

    generate_crc_table();

    crcinit_direct = crcinit;
    crc = crcinit;
    for (i = 0; i < order; i++) {
        bit = crc & 1;
        if (bit) 
		crc = crc ^ poly;
        crc = crc >> 1;
        if (bit)
		crc = crc | crchighbit;
    }
    crcinit_nondirect = crc;

    while (true) {
        char message[1024];
        printf("Enter your message: ");
        scanf("%s", message);

	gettimeofday(&t1, NULL);			//from time t1

        char rem_message[1024];
        char formed_message[1000];

        do {
            strcpy(formed_message, string_to_bin(message));
            unsigned long msg = crctablefast((unsigned char *) message, strlen(message));
            char *temp;
            temp = (char *) malloc(8192 * sizeof(char *));
            temp = itoa(msg, temp, 2);    //base = 2

            i = 0;
            char *e;
            e = (char *) malloc(8200 * sizeof(char *));
            if (strlen(temp) < 8) {
                for (i = 0; i < 8 - strlen(temp); i++) {
                    strcat(e, "0");
                }
                strcat(e, temp);
                strcpy(rem_message, e);
            }
            else {
                strcpy(rem_message, temp);
            }
            free(temp);
            free(e);

            strcat(formed_message, rem_message);
            //add error - flip bits randomly
            random_flip(formed_message, BER);
        } while (server_communicate(client_sock, formed_message) == -1);
    }

    close(client_sock);
return 0;
}
