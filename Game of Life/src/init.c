#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <game-of-life.h>

/* set everthing to zero */

void initialize_board (int *board, int M, int N) {
  int   i, j;
  
  for (i=0; i<M; i++)
    for (j=0; j<N; j++) 
      Board(i,j) = 0;
}

/* generate random table */
//generate_table is not thread safe

void generate_table (int *board, int M, int N, float threshold,int pid) {

  int   i, j;
  int counter = 0;

  //using pid variable to create different board for each task
  srand(time(NULL)+pid);

  for (i=0; i<M; i++) {
    for (j=0; j<N; j++) {
      Board(i,j) = ( (float)rand() / (float)RAND_MAX ) < threshold;
      counter += Board(i,j);
    }
  }
}

