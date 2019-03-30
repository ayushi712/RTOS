#include <stdio.h> 
#include <string.h> 
#include <pthread.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <time.h> 

#define length 5 	          // Number of coroutines variables


pthread_t thread_id[length];            // Thread for each player

long int thread_count = 0;

pthread_mutex_t thread_locks[length];   // Creating lock for each thread

pthread_mutex_t lock;		  // Main lock for critical section

char mesg[1024] = "\0";		  // Message to be sent


// Initializing all mutex
void coroutine_init()
{


	int i = 0; 
	for(i = 0; i < length; i++)
	{
		// Initialising all the thread_locks
		pthread_mutex_init(&(thread_locks[i]),NULL);
		
		// Locking all the threads initially before the game starts
		pthread_mutex_lock(&(thread_locks[i]));       
	}


	// Initializing the critical section lock
	pthread_mutex_init(&lock, NULL);   
	
	// Locking the critical section lock
	pthread_mutex_lock(&lock);	
}



// Create a co-routine by passing function pointer
int coroutine_create(void *(*fp)(void *))      //fp is function pointer

{
	// Creating each thread
	pthread_create(&(thread_id[thread_count++]), NULL, fp, NULL);

	// Critical section so that the game function initializes
	pthread_mutex_lock(&lock);
	return thread_count - 1;
}

// Sending data to a coroutine and continue execution(handle.send function)
void coroutine_send(int coroutine_id, char *msg)

{

	// Copying message to global variable
	strcpy(mesg, msg);	

	// Unlocking the thread so that the player could play the game
	pthread_mutex_unlock(&(thread_locks[coroutine_id]));		

	// Critical section so that only one player can play at a time
	pthread_mutex_lock(&lock);	

}

// Breakpoints within functions (Yield in python)
void coroutine_yield(int coroutine_id)

{

	// Allowing the unlocked player to play the game
	pthread_mutex_unlock(&lock);

	// Locking the player so that it could play only once
	pthread_mutex_lock(&(thread_locks[coroutine_id]));	

}

// Killing a co routine (closing the handle in python)
void coroutine_kill(int coroutine_id)

{

	// Killing the thread
	pthread_cancel(thread_id[coroutine_id]);	

	// Killing the mutex lock
	pthread_mutex_destroy(&(thread_locks[coroutine_id])); 

}

// A message printing Coroutine function
void* print_mesage(void *arg) 

{ 
	int coroutine_id = thread_count - 1;
	
	while(1)
	{
		coroutine_yield(coroutine_id);
		printf("Sending message\n");
		printf("Message sent by %d is: %s\n", coroutine_id, mesg);  
	}

	return NULL; 
} 

// MAIN 
int main(void) 
{ 	
	// Initializing
	coroutine_init();

	// Create coroutines 
	int coroutine1 = coroutine_create(&print_mesage);
	int coroutine2 = coroutine_create(&print_mesage);
	
	// Sending messages to co routines
	coroutine_send(coroutine1, "Dear Manoj");
	coroutine_send(coroutine2, "Dr. Anand");
	
	// Killing the routine
	coroutine_kill(coroutine1);
	coroutine_kill(coroutine2);

	return 0; 
} 
