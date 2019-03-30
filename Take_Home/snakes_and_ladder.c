#include<stdio.h> 
#include<string.h> 
#include<pthread.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<time.h> 
#include <sys/time.h>

#define length 5		// Number of coroutines variables


pthread_t thread_id[length];          // Thread for each player

long int thread_count = 0;

pthread_mutex_t thread_locks[length]; // Creating lock for each thread

pthread_mutex_t lock;		// Main lock for critical section

char mesg[length] = "\0";	// Message to be sent

// Initializing all mutex
void coroutine_init()

{
	int i = 0; 
	for(i = 0; i < length; i++)
	{

		// Initialising all the thread_locks
		pthread_mutex_init(&(thread_locks[i]), NULL);

		// Locking all the threads till all the players are created
		pthread_mutex_lock(&(thread_locks[i]));
	
	}

	// Initializing the critical section lock
	pthread_mutex_init(&lock, NULL);

	// Locking the critical section lock
	pthread_mutex_lock(&lock);

}

// Creating by coroutine by passing function pointer
int coroutine_create(void *(*fp)(void *))   //fp is a function pointer 

{

	// Creating each thread
	pthread_create(&(thread_id[thread_count++]), NULL, fp, NULL);
	
	// Critical section so that the game function initializes
	pthread_mutex_lock(&lock);
	
	return thread_count - 1;

}

// Sending data to a coroutine and continue execution(handle.send function)
void coroutine_send(int coroutine_id)

{
	
	// Unlocking the thread for game the game to be started
	pthread_mutex_unlock(&(thread_locks[coroutine_id]));

	// Critical section so that only one player can play at a time
	pthread_mutex_lock(&lock);

}

// Breakpoints within functions (Yeild in python)
void coroutine_yeild(int coroutine_id)

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

// SNAKES AND LADDER VARIABLES AND FUNCTION

int player_location[length];	// Location of each player
int snake_head[10] = {17,32,49,54,62,64,87,72,95,99}; // Start of snake
int snake_tail[10] = {7,19,26,34,40,60,36,72,75,79};  // End of snake
int ladder_start[10] = {3,10,19,21,30,38,51,65,72,80};   // Start of ladder
int ladder_end[10] = {14,85,47,34,91,67,73,91,99,95};  // End of ladder
int player_win[length];// Has a player won
int winner[length];  // Who came at what position
int no_win = 0;	// Number of winners or ranks

// Rolls the die
int dice_roll()
{
	return (rand()%6) + 1;
}

// Checks if player reached any snake or ladder
int pos_snake_or_ladder(int player_position, int *snake_or_ladder, int len)
{
	int i = 0;
	for(i = 0; i < len; i++)
	{
		if(snake_or_ladder[i] == player_position) 
		{
			return i;
		}	
	}
	return -1;
}

// Game Starts
void* game(void *arg) 
{ 
	int coroutine_id = thread_count - 1;	// Coroutine ID/ Player ID
	player_location[coroutine_id] = 0;   // Initializing players' location 
	player_win[coroutine_id] = 0;	// Initializing victory
	
	while(1)
	{
		coroutine_yeild(coroutine_id);
		printf("Current Player is: %d\n Current Position is: %d\n", coroutine_id, player_location[coroutine_id]);
		int dice_value = dice_roll();
		printf("\tValue on the dice is %d\n", dice_value);

		// If the value on the dice + current position >100, stay at current position else, move further
		if(player_location[coroutine_id] + dice_value <= 100)
		{
			player_location[coroutine_id] += dice_value;
		}

		// If reached any snake head,come to its tail's position
		if(pos_snake_or_ladder(player_location[coroutine_id],snake_head, 10) != -1)
		{
			int i = pos_snake_or_ladder(player_location[coroutine_id], snake_head, 10);
			player_location[coroutine_id] = snake_tail[i];
			printf("\tOOPs!! Current Player bit by snake\n");
		}
		
		printf("\tNow Player is at %d\n", player_location[coroutine_id]);


		// If reached a ladder,climb upto its head
		if(pos_snake_or_ladder(player_location[coroutine_id],ladder_start,8) != -1)
		{
			int i = pos_snake_or_ladder(player_location[coroutine_id], ladder_start, 10);
			player_location[coroutine_id] = ladder_end[i];	
			printf("\tYay!! Current Player climbed a ladder\n");
		}	
		
		printf("\tNow Player is at %d\n", player_location[coroutine_id]);


		// Check for win
		if(player_location[coroutine_id] == 100)
		{
			printf("Hurray!! Player %d wins!\n", coroutine_id);
			winner[no_win] = coroutine_id;// Which player won
			no_win += 1;// Incrementing ranks of winners
			player_win[coroutine_id] = 1;// Making that player state to be won
			

			while(1)
			{
				coroutine_yeild(coroutine_id);
			}
		}

		// Finding the last player after getting 3 winner players
		if(no_win == 3)
		{
			int i;
			// Finding the 4th player
			for(i = 0; i < 4; i++)
			{
				if (player_win[i] == 0)
				{
					winner[3] = i;
				}
			}
			player_win[coroutine_id] = 1;//Setting player to be won
			

			while(1)
			{
				coroutine_yeild(coroutine_id);
			}
		}
	
	}

	return NULL; 
} 

// MAIN
int main(void) 

{ 	
	srand(time(0));
	coroutine_init();
	// Stating the program

	// Creating coroutines 
	int player1 = coroutine_create(&game);
	int player2 = coroutine_create(&game);
	int player3 = coroutine_create(&game);
	int player4 = coroutine_create(&game);
	
	// Play till n-1 player has not won
	while(player_win[player1] == 0 | player_win[player2] == 0 | player_win[player3] == 0 | player_win[player4] == 0)

	{	
			if(player_win[player1] == 0)
			{
				// coroutine_send(player1, "Play");
				coroutine_send(player1);
			}
			if(player_win[player2] == 0)
			{
				// coroutine_send(player2, "Play");
				coroutine_send(player2);
			}
			if(player_win[player3] == 0)
			{
				// coroutine_send(player3, "Play");
				coroutine_send(player3);
			}
			if(player_win[player4] == 0)
			{
				// coroutine_send(player4, "Play");
				coroutine_send(player4);
			}
	}
	
	// Winners
	printf("\nWinners are:\n");
	printf("1st Winner is: player %d\n", winner[0]);
	printf("2nd Winner is: player %d\n", winner[1]);
	printf("3rd Winner is: player %d\n", winner[2]);
	printf("Loser is: player %d\n", winner[3]);

	// Kill the routines
	coroutine_kill(player1);
	coroutine_kill(player2);
	coroutine_kill(player3);
	coroutine_kill(player4);
	
	return 0; 
} 
