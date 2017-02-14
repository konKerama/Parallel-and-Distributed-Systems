#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "pthread.h"

#define MAXBINS 8

// Mutex to avoid race conditions on thread count
pthread_mutex_t activeThreadMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
  unsigned long int *morton_codes;
  unsigned long int *sorted_morton_codes;
  unsigned int *permutation_vector;
  unsigned int *index;
  unsigned int *level_record;
  int N;
  int population_threshold;
  int sft;
  int lv;
  int nThreads;
  int *activeThreads;
} radixData;


inline void swap_long(unsigned long int **x, unsigned long int **y){

  unsigned long int *tmp;
  tmp = x[0];
  x[0] = y[0];
  y[0] = tmp;

}

inline void swap(unsigned int **x, unsigned int **y){

  unsigned int *tmp;
  tmp = x[0];
  x[0] = y[0];
  y[0] = tmp;

}


void *truncated_radix_sort(void *arguments){
  radixData *args = (radixData*)arguments;
  
  unsigned long int *morton_codes = args->morton_codes;
  unsigned long int *sorted_morton_codes = args->sorted_morton_codes;
  unsigned int *permutation_vector = args->permutation_vector;
  unsigned int *index = args->index;
  unsigned int *level_record = args->level_record;
  int N = args->N;
  int population_threshold = args->population_threshold;
  int sft = args->sft;
  int lv = args->lv;
  int nThreads = args->nThreads;
  int *activeThreads = args->activeThreads;

  int BinSizes[MAXBINS] = {0};
  int BinCursor[MAXBINS] = {0};
  unsigned int *tmp_ptr;
  unsigned long int *tmp_code;

  if(N<=0){

    return NULL;
  }
  else if(N<=population_threshold || sft < 0) { // Base case. The node is a leaf

    level_record[0] = lv; // record the level of the node
    memcpy(permutation_vector, index, N*sizeof(unsigned int)); // Copy the pernutation vector
    memcpy(sorted_morton_codes, morton_codes, N*sizeof(unsigned long int)); // Copy the Morton codes 

    return NULL;
  }
  else{


    level_record[0] = lv;
    // Find which child each point belongs to 
    for(int j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      BinSizes[ii]++;
    }

    // scan prefix (must change this code)  
    int offset = 0;
    for(int i=0; i<MAXBINS; i++){
      int ss = BinSizes[i];
      BinCursor[i] = offset;
      offset += ss;
      BinSizes[i] = offset;
    }
    
    for(int j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      permutation_vector[BinCursor[ii]] = index[j];
      sorted_morton_codes[BinCursor[ii]] = morton_codes[j];
      BinCursor[ii]++;
    }

    // Initialize the thread attributes we need (joinable)
    swap(&index, &permutation_vector);
    swap_long(&morton_codes, &sorted_morton_codes);

    /* 
    If there are free threads set the recurse to a new thread
    else do it with the current thread
    */
     
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_t *threads = (pthread_t*) malloc(nThreads*sizeof(pthread_t));
    
    // t is the number of the threads tha we have created
    int t = 0;

    radixData *radixArgs = (radixData*) malloc(MAXBINS*sizeof(radixData));

    /* Call the function recursively to split the lower levels */
    for(int i=0; i<MAXBINS; i++){
      int offset = (i>0) ? BinSizes[i-1] : 0;
      int size = BinSizes[i] - offset;
      
      radixArgs[i].morton_codes = &morton_codes[offset];
      radixArgs[i].sorted_morton_codes = &sorted_morton_codes[offset];
      radixArgs[i].permutation_vector = &permutation_vector[offset];
      radixArgs[i].index = &index[offset];
      radixArgs[i].level_record = &level_record[offset];
      radixArgs[i].N = size;
      radixArgs[i].population_threshold = population_threshold;
      radixArgs[i].sft = sft-3;
      radixArgs[i].lv = lv+1;
      radixArgs[i].nThreads = nThreads;
      radixArgs[i].activeThreads = activeThreads;
      
      pthread_mutex_lock(&activeThreadMutex);
      if(*activeThreads < nThreads && i < MAXBINS - 1)   {
	(*activeThreads)++;
	pthread_mutex_unlock(&activeThreadMutex);
	
	if(pthread_create(&threads[t], &attr, truncated_radix_sort, (void*)&radixArgs[i])) {
	  printf("Error: Cant create thread in radix_sort.c\n");
	}
	t++;
      
      } else {
        pthread_mutex_unlock(&activeThreadMutex);
	truncated_radix_sort((void*)&radixArgs[i]);
      }
    
    }
    
    for(int j=0; j<t; j++) {
      if(pthread_join(threads[j], NULL)) {
        printf("Error: Cant join threads in radix_sort.c\n");
      }
    }
    
    // Update the active threads
    pthread_mutex_lock(&activeThreadMutex);
    *activeThreads = *activeThreads - t;
    pthread_mutex_unlock(&activeThreadMutex);

    free(threads);
    free(radixArgs);
  } 
}

