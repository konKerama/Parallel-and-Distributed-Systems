#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "float.h"
#include "pthread.h"

#define DIM 3

inline unsigned int compute_code(float x, float low, float step){

  return floor((x - low) / step);
}

/* Function that does the quantization */
void quantize(unsigned int *codes, float *X, float *low, float step, int start, int N){

  for(int i=start; i<N; i++){
    for(int j=0; j<DIM; j++){
      codes[i*DIM + j] = compute_code(X[i*DIM + j], low[j], step); 
    }
  }

}

typedef struct{
  unsigned int *codes;
  float *X;
  float *low;
  float step;
  int start;
  int N;
} quantize_data;

void *quantizeParrallel(void *arguments) {
  quantize_data *args = (quantize_data*) arguments;

  unsigned int *codes = args->codes;
  float *X = args->X;
  float *low = args -> low;
  float step = args -> step;
  int start = args -> start;
  int N = args -> N;

  for(int i=start; i<N; i++){
    for(int j=0; j<DIM; j++){
      codes[i*DIM + j] = compute_code(X[i*DIM + j], low[j], step);
    }
  }
  
  pthread_exit(NULL);
}

float max_range(float *x){

  float max = -FLT_MAX;
  for(int i=0; i<DIM; i++){
    if(max<x[i]){
      max = x[i];
    }
  }

  return max;

}

void compute_hash_codes(unsigned int *codes, float *X, int N,
                        int nbins, float *min, float *max,
			int nThreads){
  
  float range[DIM];
  float qstep;

  for(int i=0; i<DIM; i++){
    range[i] = fabs(max[i] - min[i]); // The range of the data
    range[i] += 0.01*range[i]; // Add somthing small to avoid having points exactly at the boundaries 
  }

  qstep = max_range(range) / nbins; // The quantization step 
  
  // Initialize the thread ids and items per thread to be assigned
  pthread_t *threads = (pthread_t*) malloc((nThreads-1) * sizeof(pthread_t)); 
  long itemsPerThread = N / nThreads;
  int t;

  // Initialize the attributes of the threads -- All Joinable for this case
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* Create an the array of arguments to be passed to each thread
   * Assign to each thread a portion of the dataset to compute the
   * hash codes
  */
  quantize_data *arguments = (quantize_data*) malloc((nThreads-1)*sizeof(quantize_data));

  for(t=0; t<nThreads-1; t++) {

    arguments[t].codes = codes;
    arguments[t].X = X;
    arguments[t].low = min;
    arguments[t].step = qstep;
    arguments[t].start = t*itemsPerThread;
    arguments[t].N = (t+1)*itemsPerThread;

    if(pthread_create(&threads[t], &attr, quantizeParrallel, (void*)&arguments[t])) {
      printf("Error: Cant create thread in hash_codes.c\n");
    }
  }
  
  // current thread fo the last repetition
  quantize(codes, X, min, qstep, (nThreads-1)*itemsPerThread, N);

  for(t=0; t<nThreads-1; t++) {
    if(pthread_join(threads[t], NULL)) {
      printf("Error: Cant join threads in hash_codes.c\n");
    }
  }

  free(threads);
  free(arguments);

}

