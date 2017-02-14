#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <omp.h>

#define DIM 3


void data_rearrangement(float *Y, float *X, 
			unsigned int *permutation_vector, 
			int N){

  int i;
  int numThreads = omp_get_max_threads();
  unsigned long chunk = N/numThreads;

  //independent so parallel
  #pragma omp parallel for schedule(dynamic,chunk) private(i) default(shared)
  for(int i=0; i<N; i++){
    memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));
  }

}
