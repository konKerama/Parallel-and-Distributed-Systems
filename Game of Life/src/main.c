/*
 * Game of Life implementation based on
 * http://www.cs.utexas.edu/users/djimenez/utsa/cs1713-3/c/life.txt
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "mpi.h"
#include "sys/time.h"
#include <omp.h>

#include <game-of-life.h>

int main (int argc, char *argv[]) {
  int   *board, *newboard;
  int nproc, pid,i;
  struct timeval startwtime, endwtime;
  double time; 
  MPI_Init( &argc, &argv);
  MPI_Comm_size( MPI_COMM_WORLD, &nproc);
  MPI_Comm_rank( MPI_COMM_WORLD, &pid);

  if (argc != 7) { // Check if the command line arguments are correct 
    printf("Usage: %s N thres disp\n"
	   "where\n"
	   "  M     : size of table\n"
	   "  N     : size of table (N x N)\n"
	   "  thres : propability of alive cell\n"
     "  t     : number of generations\n"
	   "  thr   : Number of Threads"
	   "  disp  : {1: display output, 0: hide output}\n"
	   , argv[0]);
    return (1);
  }

  // Input command line arguments
  int M =atoi(argv[1]);
  int N = atoi(argv[2]);        // Array size
  double thres = atof(argv[3]); // Propability of life cell
  int t = atoi(argv[4]);        // Number of generations 
  int thr =atoi(argv[5]);       // Number of Threads
  int disp = atoi(argv[6]);     // Display output?

  //Calculate the number of rows per Task
  int rowsPerTask=M/nproc;
  printf("Total Tasks: %d\n",nproc);
  printf("Pid: %d,Size: %dx%d with propability: %0.1f%%\n", pid, rowsPerTask, N, thres*100);
  //Set number of threads
  omp_set_num_threads(thr);
  //Synchronize the Tasks and start Time
  MPI_Barrier(MPI_COMM_WORLD);

  gettimeofday (&startwtime, NULL);

  board = NULL;
  newboard = NULL; 
  board = (int *)malloc(rowsPerTask*N*sizeof(int));
  if (board == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }
  /* second pointer for updated result */
  newboard = (int *)malloc(rowsPerTask*N*sizeof(int));
  if (newboard == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }
  //initialize_board (board, rowsPerTask, N);
  //printf("Board initialized\n");
  generate_table (board, rowsPerTask, N, thres,pid);
  printf("Board generated\n");
  
  /*
	If 1 Task just run play with openMP
	else send top and bottom run MPIplay for the middle elements
	receive the top and bottom and then run playFirstRow and playLastRow
  */
  if (nproc==1){
    for (i=0; i<t; i++) {
	  //Ignore output as board too big
      //if (disp) display_table (board, M, N);
      play (board, newboard, rowsPerTask, N);
      int *temp = newboard;
      newboard = board;
      board = temp;
    }
  }else{
    int *top,*bottom;
    top = (int *)malloc(N*sizeof(int));
    bottom = (int *)malloc(N*sizeof(int));
    int prev,next;
    if ((pid-1) <0){
      prev=nproc-1;
    }else{
      prev =pid-1;
    }
    if ((pid+1==nproc)){
      next=0;
    }else{
      next=pid+1;
    } 
    MPI_Request mpireq[4];
    MPI_Status stats[2];
    int tagTop=0;
    int tagBottom=1;
    for (i=0; i<t; i++){
      MPI_Isend(board+(rowsPerTask - 1)*N, N, MPI_INT, next, tagTop, MPI_COMM_WORLD, &mpireq[0]);
      MPI_Isend(board, N, MPI_INT, prev, tagBottom, MPI_COMM_WORLD, &mpireq[1]);   
      MPIplay(board,newboard, rowsPerTask, N);
      MPI_Irecv(top, N , MPI_INT, prev, tagTop , MPI_COMM_WORLD, &mpireq[2] );       
      MPI_Irecv(bottom, N , MPI_INT, next, tagBottom , MPI_COMM_WORLD, &mpireq[3] );
      //Wait to receive the rows
	  MPI_Waitall(2, &mpireq[2], stats);
      playFirstRow (board, newboard, top, N);
      playLastRow (board,newboard,bottom,rowsPerTask,N);
	  //Wait to send the rows before changing them
	  MPI_Waitall(2, &mpireq[0], stats);
      int *temp = newboard;
      newboard = board;
      board = temp;
    }
	//Synchronize and end time
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
  }
  gettimeofday (&endwtime, NULL);
  time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
  printf("Pid: %d,Time: %lfs, Game finished after %d generations.\n", pid,time,t);
  return(0);
}