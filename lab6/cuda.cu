#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <sys/time.h>
#include <math.h>
#include "numgen.c"

// Function to print error message and exit
__host__
void errorexit(const char *s) {
    printf("\n%s", s);	
    exit(EXIT_FAILURE);	 	
}

__device__
int isPrime(long value) {
    if (value < 2) return 0;          
    if (value == 2) return 1;         
    if (value % 2 == 0) return 0;     

    long sqroot = sqrt((double)value);
    for (long i = 3; i <= sqroot; i += 2) {
        if (value % i == 0) {
            return 0;                 
        }
    }
    return 1; // Number is prime
}

// Kernel to count primes in a given range
__global__
void countPrimes(const unsigned long int *numbers, int *result, long inputArgument) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < inputArgument) {
        result[idx] = isPrime(numbers[idx]);   // Store result of prime check in result array
    }
}

int main(int argc, char **argv) {
    Args ins__args;
    parseArgs(&ins__args, &argc, argv);       
  
    long inputArgument = ins__args.arg;       
    unsigned long int *numbers = (unsigned long int *)malloc(inputArgument * sizeof(unsigned long int));
    numgen(inputArgument, numbers);           

    int threadsinblock = 1024;             
    int blocksingrid = (inputArgument + threadsinblock - 1) / threadsinblock;  // Calculate number of blocks in grid
    
    // Allocate memory on the host
    int *h_results = (int *)malloc(inputArgument * sizeof(int));
    if (!h_results) errorexit("Error allocating memory on the host");

    // Allocate memory on the GPU
    unsigned long int *d_numbers = NULL;
    int *d_results = NULL;
      // memory for input number
    if (cudaSuccess != cudaMalloc((void **)&d_numbers, inputArgument * sizeof(unsigned long int)))
        errorexit("Error allocating memory on the GPU for numbers");
      // memory for results
    if (cudaSuccess != cudaMalloc((void **)&d_results, inputArgument * sizeof(int)))
        errorexit("Error allocating memory on the GPU for results");

    // Copy data from host to device
    if (cudaSuccess != cudaMemcpy(d_numbers, numbers, inputArgument * sizeof(unsigned long int), cudaMemcpyHostToDevice))
        errorexit("Error copying numbers to GPU");

    // count excetion time - begin
    struct timeval ins__tstart, ins__tstop;
    gettimeofday(&ins__tstart, NULL);

    // Launch the kernel to count primes
    countPrimes<<<blocksingrid, threadsinblock>>>(d_numbers, d_results, inputArgument);
    if (cudaSuccess != cudaGetLastError())
        errorexit("Error during kernel launch");

    // Copy results from GPU to host
    if (cudaSuccess != cudaMemcpy(h_results, d_results, inputArgument * sizeof(int), cudaMemcpyDeviceToHost))
        errorexit("Error copying results from GPU");

    // On the host - Sum the results
    long long result = 0;
    for (long i = 0; i < inputArgument; i++) {
        result += h_results[i];
    }

    printf("\nThe final result is %lld\n", result);

    // Free allocated memory
    free(numbers);
    free(h_results);
    if (cudaSuccess != cudaFree(d_numbers))
        errorexit("Error when deallocating space on the GPU for numbers");
    if (cudaSuccess != cudaFree(d_results))
        errorexit("Error when deallocating space on the GPU for results");


    // Stop timing the kernel execution
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
    
    return 0;
}
