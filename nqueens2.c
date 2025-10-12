/***********************************************************
 n-Queens Puzzle Solver
 
 Originally created for CSC471 - Parallel Programming, at
 University of Advancing Technology in Tempe, AZ
 
 Featuring a dual-mode algorithm that can execute either
 sequentially or as a multi-threaded parallel program.
 Requires standard C/UNIX libraries and POSIX threads.
 
 #defines that can be set:
 N = number of queens and size of board to solve for
 PARALLEL = 0/1 flag to set parallel mode
 SOLUTIONS = space in memory to hold array of solutions
 PRINTSOLS = 0/1 flag if solutions are to be displayed.
 
 By Stephen Sviatko - ssviatko@hotmail.com
 04/May/2009 - v0.5.0
 
 (C) 2009 Stephen Sviatko & Associates LLC
 Permission is granted to use this code in whole or in part,
 as long as the original author is notified and is properly
 credited in any derivative work.
 ************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

// user-defineable settings
#define N		14
#define PARALLEL	1
#define SOLUTIONS	370000
#define PRINTSOLS	0

// used by the algorithm
#define QUEEN		1
#define NOQUEEN		0
#define ATTACK		1

/*
typedef struct {
	pthread_mutex_t sol_lock; // lock, for adding solutions in parallel mode
	int next_sol; // next open slot
	int unique_sol; // number of unique solutions
	unsigned int board[SOLUTIONS][N][N]; // the solved boards
	int is_unique[SOLUTIONS]; // uniqueness flag
} Solutions;

Solutions sol;
*/

typedef struct {
	int instance; // NOT threadid, but instance from 0 to (N - 1)
	pthread_t thread; // tid
	int curcol; // current column we are working on
	int rowforcol[N]; // current row in each column we have stepped through
	unsigned int b[N][N]; // working board
	unsigned int a[N][N]; // attack marks
	int finished; // termination flag
	int threadsol; // solutions this particular thread found
} Thread_Data;

Thread_Data td[N / 2 + 1];

void calc_attack(int which_instance)
{
	int x, y, xx, yy;

	// scan the board for queens and mark all their attacked squares
	for (x = 0; x < N; x++)
		for (y = 0; y < N; y++)
			if (td[which_instance].b[x][y] == QUEEN)
				for (xx = 0; xx < N; xx++)
					for (yy = 0; yy < N; yy++)
						if ((abs(x - xx) == abs(y - yy)) || ((x == xx) || (y == yy))) // grab row/column/diagonal all at once
							td[which_instance].a[xx][yy] = ATTACK;
}

/*
void print_unique_solutions(void)
{
	int i, x, y;

	if (sol.next_sol > 0)
		for (i = 0; i < sol.next_sol; i++)
			if (sol.is_unique[i])
			{
				printf("Unique Solution (index #%d)\n---\n", i + 1);
				for (y = 0; y < N; y++)
				{
					for (x = 0; x < N; x++)
						sol.board[i][x][y] == QUEEN ? printf(" Q") : printf(" -");
					printf("\n");
				}
				printf("\n");
			}
}
*/

void *calc_tf(void *arg)
{
	int i, j, k, x, y, xx, yy;
	int this_is_unique, is_same;
	int our_solution;
	unsigned int mods[7][N][N]; // rotated/mirrored boards to test

	Thread_Data *our_td = (Thread_Data *)arg;
	int middlestart; // for the sequential algorithm, to determine if we started off with a queen in the "middle" row of column 0 on an odd sized board
	
	while (!our_td->finished)
	{
		// if there is a queen placed in the current column, pick her up
		if (our_td->rowforcol[our_td->curcol] != -1)
		{
			our_td->b[our_td->curcol][our_td->rowforcol[our_td->curcol]] = NOQUEEN;
			memset(our_td->a, 0, N * N * sizeof(unsigned int)); // clear attack marks
			calc_attack(our_td->instance);
		}
		
		// place queen in next unattacked row in current column
		do our_td->rowforcol[our_td->curcol]++;
		while ((our_td->a[our_td->curcol][our_td->rowforcol[our_td->curcol]] == ATTACK) && (our_td->rowforcol[our_td->curcol] < N));
		
		if (our_td->curcol > 0 ? our_td->rowforcol[our_td->curcol] < N : our_td->rowforcol[our_td->curcol] < ((N % 2 == 0) ? N / 2 : N / 2 + 1))
		/* explanation of this if statement: if we are in a column > 0, we want to check if the row is < N (doing the entire column)
		 if we are in column 0 (which will only happen if we are running in sequential mode), we want to check if the row is < N/2
		 (or less than N/2 + the "middle" row, if we are on an odd sized board) */
		{
			// if we haven't run off the bottom of the board, place her
			our_td->b[our_td->curcol][our_td->rowforcol[our_td->curcol]] = QUEEN;
			calc_attack(our_td->instance);
			
			// set the middlestart flag appropriately if we just placed her in column 0, we are on an odd-sized board, and we are in the "middle" row
			if (our_td->curcol == 0)
				middlestart = ((N % 2 == 1) && (our_td->rowforcol[0] == N / 2)) ? 1 : 0; // == N / 2 instead of N / 2 + 1, since rowforcol[] is zero based
			
			// advance to next column
			our_td->curcol++;
			if (our_td->curcol == N) // if we ran off the right side of the board...
			{
/*
				// we got a solution
				if (PARALLEL)
					pthread_mutex_lock(&sol.sol_lock);
				// make sure we have enough solution pidgeonholes to insert this
				if (sol.next_sol >= SOLUTIONS)
				{
					printf("Out of space in solutions array, please increase allocation.\n");
					exit(-1);
				}
				memcpy(&sol.board[sol.next_sol], our_td->b, N * N * sizeof(unsigned int)); // copy our board into the solutions array
				sol.is_unique[sol.next_sol] = 1; // mark it as unique, whether it is tested or not
				our_solution = sol.next_sol++; // so we know which solution was ours after we unlock the mutex; sol.next_sol may be changed by another thread
				if (PARALLEL)
					pthread_mutex_unlock(&sol.sol_lock);
*/
				our_td->threadsol++; // record that this thread found a solution
				
				// if we are running in sequential mode, bump the solutions count by 2 (except when we started with a queen in the "middle" row of column 0)
				if (!PARALLEL && !middlestart)
					our_td->threadsol++;
/*	
				// test for uniqueness
				if (our_solution > 0) // only if it isn't the first solution found so far
				{
					// make a matrix of modified boards based on our recently-found solution
					// 0 deg mirrored
					for (x = 0; x < N; x++)
						for (y = 0; y < N; y++)
							mods[0][N - x - 1][y] = sol.board[our_solution][x][y];
					// 90 deg (counterclockwise)
					for (x = 0; x < N; x++)
						for (y = 0; y < N; y++)
							mods[1][y][N - x - 1] = sol.board[our_solution][x][y];
					// 90 deg mirrored
					for (x = 0; x < N; x++)
						for (y = 0; y < N; y++)
							mods[2][N - x - 1][y] = mods[1][x][y];
					// 180 deg (upside down)
					for (x = 0; x < N; x++)
						for (y = 0; y < N; y++)
							mods[3][N - x - 1][N - y - 1] = sol.board[our_solution][x][y];
					// 180 deg mirrored
					for (x = 0; x < N; x++)
						for (y = 0; y < N; y++)
							mods[4][N - x - 1][y] = mods[3][x][y];
					// 270 deg (counterclockwise)
					for (x = 0; x < N; x++)
						for (y = 0; y < N; y++)
							mods[5][N - y - 1][x] = sol.board[our_solution][x][y];
					// 270 deg mirrored
					for (x = 0; x < N; x++)
						for (y = 0; y < N; y++)
							mods[6][N - x - 1][y] = mods[5][x][y];
					
					// iterate through all boards from 0 to our_solution - 1 to see if they match any of our mods
					for (i = 0; i < our_solution; i++)
					{
						// test the to see if it equals any of our 7 modifications
						this_is_unique = 1;
						for (k = 0; k < 7; k++)
						{
							is_same = 1;
							for (xx = 0; xx < N; xx++)
								for (yy = 0; yy < N; yy++)
									if (mods[k][xx][yy] != sol.board[i][xx][yy])
										is_same = 0;
							if (is_same > 0) // found one that's the same..
							{
								this_is_unique = 0;
								break;
							}
						}
						if (this_is_unique == 0)
						{
							if (PARALLEL)
								pthread_mutex_lock(&sol.sol_lock);
							sol.is_unique[our_solution] = 0;
							if (PARALLEL)
								pthread_mutex_unlock(&sol.sol_lock);
							break;
						} // if this_is_unique
					} // for 0 to our_solution - 1 (compare the mods to the existing solutions)
				} // if our_solution > 0 (test for uniqueness)
*/				
				// step back
				our_td->curcol--;
				continue;
			} // if our_td->curcol == N (got a solution)
		} // if we didn't run off the bottom of the board
		else
		{
			// no more rows in this column (ran off bottom), so step backwards
			our_td->rowforcol[our_td->curcol] = -1; // reset this row pointer
			our_td->curcol--;
			
			// if we have to step back to column 0 (for sequential) or 1 (parallel), we are finished
			if (our_td->curcol < PARALLEL ? 1 : 0)
				our_td->finished = 1;
		}
	}
	if (PARALLEL)
		pthread_exit(NULL);
	else
		return NULL;
}
	
int main (int argc, char **argv)
{
	int i, j, res; // result variable for pthread calls
	int solutions = 0;
	int num_threads = (N % 2 == 0) ? N / 2 : N / 2 + 1;
	struct timeval start_time, end_time;
	
	gettimeofday(&start_time, NULL);

/*	
	sol.next_sol = 0; // init solutions pointer
	if (PARALLEL)
	{
		res = pthread_mutex_init(&sol.sol_lock, NULL);
		if (res != 0)
		{
			perror("Fatal Error: Can't initialize solutions mutex!");
			exit(-1);
		}
	}
*/
	
	// set up thread data
	if (PARALLEL)
	{
		for (i = 0; i < num_threads; i++)
		{
			td[i].instance = i; // instance is the index
			td[i].curcol = 1; // start working in the second column
			for (j = 0; j < N; j++)
				td[i].rowforcol[j] = -1; // no queen currently placed in any of the columns
			memset(td[i].b, 0, N * N * sizeof(unsigned int)); // clear the board
			memset(td[i].a, 0, N * N * sizeof(unsigned int)); // clear attack marks
			
			// place a queen in the Nth row of column 0 on each thread's board
			td[i].b[0][i] = QUEEN;
			calc_attack(i);
			
			td[i].finished = 0;
			td[i].threadsol = 0;
		}
	}
	else
	{
		// sequential setup
		td[0].instance = 0;
		td[0].curcol = 0; // start working in the first column
		for (j = 0; j < N; j++)
			td[0].rowforcol[j] = -1; // no queen currently placed in any of the columns
		memset(td[0].b, 0, N * N * sizeof(unsigned int)); // clear the board
		memset(td[0].a, 0, N * N * sizeof(unsigned int)); // clear attack marks
		td[0].finished = 0;
		td[0].threadsol = 0;
	}
	
	// print environment
//	printf("Size of solutions matrix: %ld bytes (anticipating %d solutions by calculation)\n", sizeof(sol), SOLUTIONS);
	printf("Using %s algorithm\n", PARALLEL ? "parallel" : "sequential");
	printf("Calculating n = %d queens, please wait...\n", N);

	if (PARALLEL)
	{
		// start the threads
		for (i = 0; i < num_threads; i++)
		{
			res = pthread_create(&td[i].thread, NULL, calc_tf, &td[i]);
			if (res != 0)
			{
				perror("Fatal Error: Thread creation failed!");
				exit(-1);
			}
		}
		
		// join the threads
		for (i = 0; i < num_threads; i++)
		{
			res = pthread_join(td[i].thread, NULL);
			if (res != 0)
			{
				perror("Fatal Error: Can't join thread!");
				exit(-1);
			}
			else
			{
				printf("Joined with thread %d (threadid 0x%016lX), found %d solutions.\n", i, td[i].thread, td[i].threadsol);
			}
		}
	}
	else
	{
		calc_tf(&td[0]);
	}
/*	
	// count our unique solutions
	for (i = 0; i < sol.next_sol; i++)
		if (sol.is_unique[i])
			sol.unique_sol++;
*/
	gettimeofday(&end_time, NULL);
/*	
	if (PRINTSOLS)
		print_unique_solutions();

	printf("Generated %d solutions by calculation.\n", sol.next_sol);
*/	
	if (PARALLEL)
	{
		// bump total solutions count to reflect symmetrical finds (N/2 to N threads which we didn't execute)
		for (i = N - 1; i >= num_threads; i--)
			solutions += td[N - i - 1].threadsol;
	}
	else
	{
		// make total solutions equal the number in td[0].threadsol, that we manually adjusted in the sequential version of the calc function.
		solutions = td[0].threadsol;
	}
	
	printf("Found %d total solutions.\n", solutions);
//	printf("Found %d unique solutions.\n", sol.unique_sol);
	printf("Elapsed time: %ld seconds %ld usecs.\n",
		   end_time.tv_sec - start_time.tv_sec - ((end_time.tv_usec - start_time.tv_usec < 0) ? 1 : 0), // subtract 1 if there was a usec rollover
		   end_time.tv_usec - start_time.tv_usec + ((end_time.tv_usec - start_time.tv_usec < 0) ? 1000000 : 0) // bump usecs by 1 million usec for rollover
	);
/*	
	// clean up
	if (PARALLEL)
		pthread_mutex_destroy(&sol.sol_lock);
*/	
	return 0;
}
