#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "pthread.h"

#define DIM 3

typedef struct{
  float *Y;
  float *X;
  unsigned int *permutation_vector;
  int start;
  int itemsPerThread;
} rearrData;

void *data_rearrangementParallel(void* arguments) {
  rearrData *args = (rearrData*) arguments;

  float *Y = args->Y;
  float *X = args->X;
  unsigned int *permutation_vector = args->permutation_vector;
  int start = args->start;
  int itemsPerThread = args->itemsPerThread;

  for(int i=start; i<itemsPerThread; i++) {
    memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));
  }
  
  pthread_exit(NULL);
}


void data_rearrangement(float *Y, float *X, 
			unsigned int *permutation_vector, 
			int N, int nThreads){

  // Initialize the thread ids
  pthread_t *threads = (pthread_t*) malloc((nThreads-1)*sizeof(pthread_t));
  long itemsPerThread = N / nThreads;
  int t;

  // Set threads to be joinable
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // create an array of rearrData arguments
  rearrData *arguments = (rearrData*) malloc((nThreads-1)*sizeof(rearrData));

  for(t=0; t<nThreads-1;t++) {

    arguments[t].Y = Y;
    arguments[t].X = X;
    arguments[t].permutation_vector = permutation_vector;
    arguments[t].start = t*itemsPerThread;
    arguments[t].itemsPerThread = (t+1)*itemsPerThread;
    
    if(pthread_create(&threads[t], &attr, data_rearrangementParallel, (void*)&arguments[t])) {
      printf("Error:Cant create thread in data_rearrangement.c\n");
    }

  }
  
  //the last repetion is done by the current thread
  for(int i=(nThreads-1)*itemsPerThread; i<N; i++){
    memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));
  }
 
  for(t=0; t<nThreads-1; t++) {
    if(pthread_join(threads[t], NULL)) {
      printf("Error: Cant join threads in data_rearrangement.c\n");
    }
  }
  
  free(threads);
  free(arguments);

}
