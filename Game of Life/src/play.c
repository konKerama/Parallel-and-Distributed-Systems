#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "mpi.h"
#include <omp.h>

#include <game-of-life.h>

void play (int *board, int *newboard, int M, int N){
  int   i, j, a;
  #pragma omp parallel for private(j,a) 
  for (i=0; i<M; i++){
    for (j=0; j<N; j++) {
      a = adjacent_to (board, i, j, M, N);
      if (a == 2) NewBoard(i,j) = Board(i,j);
      if (a == 3) NewBoard(i,j) = 1;
      if (a < 2) NewBoard(i,j) = 0;
      if (a > 3) NewBoard(i,j) = 0;
    }
  }
}

//MPIplay computes only the middle elements 
void MPIplay (int *board, int *newboard, int M, int N){

  int   i, j, a;
  #pragma omp parallel for private(j,a) 
  for (i=1; i<M-1; i++)
    for (j=0; j<N; j++) {
      a = adjacent_to (board, i, j, M, N);
      if (a == 2) NewBoard(i,j) = Board(i,j);
      if (a == 3) NewBoard(i,j) = 1;
      if (a < 2) NewBoard(i,j) = 0;
      if (a > 3) NewBoard(i,j) = 0;
   }
}
//playFirstRow computes the top elements of current board
void playFirstRow (int *board, int *newboard, int *top, int N){
  int i;
  #pragma omp parallel for private(i)
  for (i=0;i<N;i++){
    topAdjacent_to(board, newboard, top, i, N);
  }
}
//playLastRow computes the bottom elements of current board
void playLastRow (int *board, int *newboard, int *bottom, int M, int N){
  int i;
  #pragma omp parallel for private(i)
  for (i=0;i<N;i++){
    bottomAdjacent_to(board, newboard, bottom, i, M, N);
  }
}

