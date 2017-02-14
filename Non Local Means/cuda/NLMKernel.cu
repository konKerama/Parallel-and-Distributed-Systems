#include <math.h>
#include <stdio.h>

// Array access macros
#define INPUT(i,j) imgBef[(i)*n + j]
#define OUTPUT(i,j) imgAfter[(i)*n + j]
#define fNi(i,j) fNi[(i)*patchSize + j]
#define fNj(i,j) fNj[(i)*patchSize + j]
#define fN(i,j) fN[(i)*patchSize + j]
#define H(i,j) H[(i)*patchSize + j]
#define PATCH(i,j) patch[(i)*blockDim.y + j]
#define OTHER_PATCH(i,j) other_patch[(i)*blockDim.y + j]

/* If out of block -> get fNi from global memory
   Else get from patch (shared)
   Out of block happens when: k or l <= padSize  OR
   if k > blockDim.x + padSize or if l > blockDim.y + padSize
*/

__device__ int isInBlock(int k, int l, int padSize, int a, int b){

	if (k<0 || l<0 || k>a || l>b){
		return 0;
	}
	else {
		return 1;
	}
}

/* Calculates the norm between two vectors based on the gaussian matrix H for
   a predetermined patch size.
   */
__device__ float calcNorm(float *fNi, float *fNj, float *H, int patchSize){
	float sum =0;
	for (int k=0; k< patchSize; k++){
		for (int l=0; l<patchSize; l++){
			sum+=(fNi(k,l)-fNj(k,l))*(fNi(k,l)-fNj(k,l))*H(k,l);
		}
	}
	return sum;
}

/* 
 Checks if targetted pixel at the edges of the final blocks is outside of the image.
*/
__device__ int pixelOutofImg(int x, int y, int m, int n){

	if (x > m || y > n){
		return 1;
	}
	else{
		return 0;
	}

}

__device__ void computeFN(float * fN, int i, int j, int n, float * patch, float const * imgBef, int padSize, int patchSize, int currentBlock_X, int currentBlock_Y, int blockDimX, int blockDimY ){
	
	int a,b,k,l,e=0,f=0;
	for (k=i-padSize; k<=i+padSize; k++){
				for (l = j-padSize; l<=j+padSize; j++){ 
					a = k - currentBlock_X * blockDimX; // a e [0,patchSize)
					b = l - currentBlock_Y * blockDimY; // b e [0,patchSize)

					if (isInBlock(a, b, padSize, blockDimX, blockDimY)){
						fN(e,f) = PATCH(a,b);
					}
					else{
						fN(e,f) = INPUT(k,l);
					}
					f++;
					if (f==patchSize){
						f=0;
						e++;
					}
				}
			}	

}

__global__ void cudaNonLocalMeans(float const *imgBef, float *imgAfter, float *H, float filtSigma,
                                    int m, int n, int padSize) {
	// Get pixel (x,y) in input

	__shared__ extern float patch[];
	float *other_patch = patch + blockDim.x*blockDim.y;
	int currentBlock_X = blockIdx.x; // [0,numBlocksX)
	int currentBlock_Y = blockIdx.y; // [0,numBlocksY)

	int patchSize = 2*padSize+1;
	int notInPaddedArea=1;
	//float *other_patch = patch +
	int i = currentBlock_X * blockDim.x + threadIdx.x;
	int j = currentBlock_Y * blockDim.y + threadIdx.y;
	// nBlocks_X,Y is the number of blocks per axis
	int nBlocks_X = gridDim.x;
	int nBlocks_Y = gridDim.y;

	float Z=0;
	float exponent=0;
	float pixel=0;
	float *fNi, *fNj;

	// if i or j less than 2 or greater than 65 they are outside of the image.
	if (i < padSize || j < padSize || i > m-padSize-1 || j > n-padSize-1) notInPaddedArea=0;

	if (i < m && j < n){ // INSIDE IMAGE
		if (notInPaddedArea){
			PATCH(threadIdx.x, threadIdx.y) = INPUT(i,j);
			// fNi calculation
			computeFN(fNi, i, j, n, patch,imgBef,padSize,patchSize,currentBlock_X, currentBlock_Y, blockDim.x, blockDim.y);
			// fNj calculation for CURRENT block only.
			for (int row = currentBlock_X*blockDim.x ; row<(currentBlock_X+1)*blockDim.x; row++){
				for (int col = currentBlock_Y*blockDim.y ; col<(currentBlock_Y+1)*blockDim.y; col++){
					//if (row > m-padSize || col > n-padSize || row < padSize || col < padSize )continue;
					computeFN(fNj, row, col, n, patch,imgBef,padSize,patchSize,currentBlock_X, currentBlock_Y, blockDim.x, blockDim.y);
					exponent = calcNorm(fNi, fNj, H, patchSize);
					exponent/=filtSigma;
					Z+=exp(-exponent);
					pixel += exp(-exponent)*INPUT(row,col);
				}
			}
		}
	}
	__syncthreads();

	/*
	  So far the pixel has been affected only by the values of its own block.
	  The following code implements the effect of the rest of the blocks on
	  the pixel.
	*/

	int blockNoX, blockNoY;
	// For each block other than the current 
	for (blockNoX = 0; blockNoX < nBlocks_X ; blockNoX++){
		for (blockNoY = 0; blockNoY < nBlocks_Y ; blockNoY++){
			if (blockNoX!=currentBlock_X && blockNoY!=currentBlock_Y){
				// Save block to shared
				// For each pixel of the block copy to shared (other_patch)
				if (!(pixelOutofImg(blockNoX * blockDim.x + threadIdx.x, blockNoY * blockDim.y + threadIdx.y, m, n))){
					OTHER_PATCH(threadIdx.x,threadIdx.y)=INPUT(blockNoX * blockDim.x + threadIdx.x , blockNoY * blockDim.y + threadIdx.y);
				}
				__syncthreads();
				// If the pixel is inside the current block compute from shared, otherwise from global. 
				if (notInPaddedArea){
					for (int row = blockNoX*blockDim.x ; row<(blockNoX+1)*blockDim.x; row++){
						for (int col = blockNoY*blockDim.y ; col<(blockNoY+1)*blockDim.y; col++){
							if (row > m-padSize || col > n-padSize) continue;
							computeFN(fNj, row, col, n, other_patch,imgBef,padSize,patchSize,blockNoX,blockNoY, blockDim.x, blockDim.y);
							exponent = calcNorm(fNi, fNj, H, patchSize);
							exponent/=filtSigma;
							Z+=exp(-exponent);
							pixel += exp(-exponent)*INPUT(row,col);

						}
					}
				}
			}
		}
	}
	/* Finally since the total effect of the other pixels on the current pixel
	   has been calculated, pixel is divided by the finalized Z matrix ending up with its
	   final denoised version.
	*/	
	if (notInPaddedArea){
		OUTPUT(i,j) = pixel/Z;	
	}else{
		OUTPUT(i,j) = 1;
	}
	
}		
