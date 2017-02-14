#include "stdio.h"
#include "stdlib.h"
#include "sys/time.h"
#include "utils.h"
#include "pthread.h"

#define DIM 3

// Define the struct we need to pass arguments to modified radix sort
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


int main(int argc, char** argv){

  // Time counting variables 
  struct timeval startwtime, endwtime;

  if (argc != 7) { // Check if the command line arguments are correct 
    printf("Usage: %s N dist pop rep P\n"
	   "where\n"
	   "N    : number of points\n"
           "Thr  : number of threads\n"
	   "dist : distribution code (0-cube, 1-sphere)\n"
	   "pop  : population threshold\n"
	   "rep  : repetitions\n"
	   "L    : maximum tree height\n", argv[0]);
    return (1);
  }
	
  // Input command line arguments
  int N = atoi(argv[1]); // Number of points
  int Thr = atoi(argv[2]); // maximum number of threads
  int dist = atoi(argv[3]); // Distribution identifier 
  int population_threshold = atoi(argv[4]); // populatiton threshold
  int repeat = atoi(argv[5]); // number of independent runs
  int maxlev = atoi(argv[6]); // maximum tree height

  printf("Max threads: %d\n", Thr);


  printf("Running for %d particles with maximum height: %d\n", N, maxlev);

  float *X = (float *) malloc(N*DIM*sizeof(float));
  float *Y = (float *) malloc(N*DIM*sizeof(float));

  unsigned int *hash_codes = (unsigned int *) malloc(DIM*N*sizeof(unsigned int));
  unsigned long int *morton_codes = (unsigned long int *) malloc(N*sizeof(unsigned long int));
  unsigned long int *sorted_morton_codes = (unsigned long int *) malloc(N*sizeof(unsigned long int));
  unsigned int *permutation_vector = (unsigned int *) malloc(N*sizeof(unsigned int)); 
  unsigned int *index = (unsigned int *) malloc(N*sizeof(unsigned int));
  unsigned int *level_record = (unsigned int *) calloc(N,sizeof(unsigned int)); // record of the leaf of the tree and their level

  // initialize the index
  for(int i=0; i<N; i++){
    index[i] = i;
  }

  /* Generate a 3-dimensional data distribution */
  create_dataset(X, N, dist);

  /* Find the boundaries of the space */
  float max[DIM], min[DIM];
  find_max(max, X, N);
  find_min(min, X, N);

  int nbins = (1 << maxlev); // maximum number of boxes at the leaf level

  // Independent runs
  for(int it = 0; it<repeat; it++){

    gettimeofday (&startwtime, NULL); 
  
    compute_hash_codes(hash_codes, X, N, nbins, min, max, Thr); // compute the hash codes

    gettimeofday (&endwtime, NULL);

    double hash_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
     
    printf("Time to compute the hash codes            : %fs\n", hash_time);


    gettimeofday (&startwtime, NULL); 

    morton_encoding(morton_codes, hash_codes, N, maxlev, Thr); // computes the Morton codes of the particles

    gettimeofday (&endwtime, NULL);


    double morton_encoding_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);


    printf("Time to compute the morton encoding       : %fs\n", morton_encoding_time);

    /* Pass the arguments to modified radix sort
    * using the radixData struct
    */
    
    radixData arguments;
    int *activeThreads = (int*)malloc(sizeof(int));
    *activeThreads = 1;
    
    arguments.morton_codes = morton_codes;
    arguments.sorted_morton_codes = sorted_morton_codes;
    arguments.permutation_vector = permutation_vector;
    arguments.index = index;
    arguments.level_record = level_record;
    arguments.N = N;
    arguments.population_threshold = population_threshold;
    arguments.sft = 3*(maxlev-1);
    arguments.lv = 0;
    arguments.nThreads = Thr;
    arguments.activeThreads = activeThreads;

    gettimeofday (&startwtime, NULL); 

    // Truncated msd radix sort
    truncated_radix_sort((void*)&arguments);

    gettimeofday (&endwtime, NULL);
    
    //printf("Threads left after execution %d\n", *activeThreads);

    free(activeThreads);

    double sort_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

    printf("Time for the truncated radix sort         : %fs\n", sort_time);

    gettimeofday (&startwtime, NULL); 

    // Data rearrangement
    data_rearrangement(Y, X, permutation_vector, N, Thr);

    gettimeofday (&endwtime, NULL);


    double rearrange_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    

    printf("Time to rearrange the particles in memory : %fs\n", rearrange_time);

    /* The following code is for verification */ 
    // Check if every point is assigned to one leaf of the tree
    int pass = check_index(permutation_vector, N); 

    if(pass){
      printf("Index test PASS\n");
    }
    else{
      printf("Index test FAIL\n");
    }

    // Check is all particles that are in the same box have the same encoding. 
    pass = check_codes(Y, sorted_morton_codes, 
		       level_record, N, maxlev);

    if(pass){
      printf("Encoding test PASS\n");
    }
    else{
      printf("Encoding test FAIL\n");
    }

  }

  /* clear memory */
  free(X);
  free(Y);
  free(hash_codes);
  free(morton_codes);
  free(sorted_morton_codes);
  free(permutation_vector);
  free(index);
  free(level_record);
}





