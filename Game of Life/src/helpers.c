#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <game-of-life.h>

/* add to a width index, wrapping around like a cylinder */

int xadd (int i, int a, int N) {
  i += a;
  while (i < 0) i += N;
  while (i >= N) i -= N;
  return i;
}

/* add to a height index, wrapping around */

int yadd (int i, int a, int N) {
  i += a;
  while (i < 0) i += N;
  while (i >= N) i -= N;
  return i;
}

/* return the number of on cells adjacent to the i,j cell */

int adjacent_to (int *board, int i, int j, int M,int N) {
  int   k, l, count;

  count = 0;

  /* go around the cell */

  for (k=-1; k<=1; k++)
    for (l=-1; l<=1; l++)
      /* only count if at least one of k,l isn't zero */
      if (k || l)
        if (Board(xadd(i,k,M),yadd(j,l,N))) count++;
  return count;
}
/*
topAdjacent_to and bottomAdjacent_to use *top and *bottom
pointers for each element outside the current board
*/
void topAdjacent_to(int *board, int *newboard, int *top, int pos, int N){
  int k,l;
  int count;
  int a;
  count=0;
  for (k=-1; k<=1; k++){
    for(l=-1; l<=1; l++){
     if (k || l){
        a=k;
       if (a<0){
          if (top(yadd(pos,l,N))) count++;
        }else{
          if (Board(a,yadd(pos,l,N))) count++;
        }
      }
    }
  }
  if (count == 2) NewBoard(0,pos) = Board(0,pos);
  if (count == 3) NewBoard(0,pos) = 1;
  if (count < 2) NewBoard(0,pos) = 0;
  if (count > 3) NewBoard(0,pos) = 0;
}

void bottomAdjacent_to(int *board, int *newboard, int *bottom, int pos, int M,int N){
  int k,l;
  int count;
  int a;
  count=0;
  for (k=-1; k<=1; k++){
    for(l=-1; l<=1; l++){
     if (k || l){
        a=(M-1)+k;
       if (k==1){
          if (bottom(yadd(pos,l,N))) count++;
        }else{
          if (Board(a,yadd(pos,l,N))) count++;
        }
      }
    }
  }
  if (count == 2) NewBoard(M-1,pos) = Board(0,pos);
  if (count == 3) NewBoard(M-1,pos) = 1;
  if (count < 2) NewBoard(M-1,pos) = 0;
  if (count > 3) NewBoard(M-1,pos) = 0;
}
