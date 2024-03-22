#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>
#include "numgen.c"
#define RANGE_SIZE 5
#define DATA 0
#define RESULT 1
#define FINISH 2

#define DEBUG

int is_prime(int number) {
    int sqr = sqrt(number);
    int is_prime = 1;

    for(int i=2; i<=sqr; i++) {
        if(number % i == 0) {
            is_prime = 0;
            break;
        }
    }

    return is_prime;
}

int count_primes(int* arr, int array_size) {
    int result = 0;
    for(int i=0; i<array_size; i++) {
        result += is_prime(arr[i]);
    }
    return result;
}

int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;

  int myrank,nproc;
  unsigned long int *numbers;

  int range_limits[2];
  int sub_numbers[RANGE_SIZE];
  int result = 0;
  int temp_result = 0;
  int end_of_arrray = 0;

  MPI_Status status;

  MPI_Init(&argc,&argv);

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);


  if(!myrank){
      #ifdef DEBUG
      printf("generating numbers in MASTER");
      #endif

      gettimeofday(&ins__tstart, NULL);
      numbers = (unsigned long int*)malloc((inputArgument + RANGE_SIZE) * sizeof(unsigned long int));
      numgen(inputArgument, numbers);

      range_limits[0] = 0;
      int i;
      for(int i=1; i<nproc; i++) {
          // set range limits
          range_limits[1]=range_limits[0] + RANGE_SIZE;

          // create the sub array
          for(int j=range_limits[0], k=0; j<range_limits[1]; j++, k++) {
              sub_numbers[k] = numbers[j];
          }

          #ifdef DEBUG
          printf("\nMaster --> %d: ", i);
          for(int i=0; i<RANGE_SIZE; i++) {
              printf("%u, ", sub_numbers[i]);
          }
          #endif

          MPI_Send(sub_numbers, RANGE_SIZE, MPI_INTEGER, i, DATA, MPI_COMM_WORLD);

          range_limits[0]=range_limits[1];
      }

      do {
        // distribute remaining subranges to the processes which have completed their parts
          MPI_Recv(&temp_result,1,MPI_INTEGER, MPI_ANY_SOURCE, RESULT, MPI_COMM_WORLD, &status);
          result += temp_result;

          #ifdef DEBUG
          printf("\nMaster received result %d from process %d",temp_result,status.MPI_SOURCE);
          #endif

          range_limits[1]=range_limits[0] + RANGE_SIZE;

          for(int j=range_limits[0], k=0; j<range_limits[1]; j++, k++) {
              sub_numbers[k] = numbers[j];
          }

          #ifdef DEBUG
          printf("\nMaster giving more --> %d: ", status.MPI_SOURCE);
          for(int i=0; i<RANGE_SIZE; i++) {
              printf("%u, ", sub_numbers[i]);
          }
          #endif

          MPI_Send(sub_numbers, RANGE_SIZE, MPI_INTEGER, status.MPI_SOURCE, DATA, MPI_COMM_WORLD);

          range_limits[0] = range_limits[1];
      } while(range_limits[1] < inputArgument);


      // recieve results

      for(int i=0; i<nproc-1; i++) {
          MPI_Recv(&temp_result, 1, MPI_INTEGER, MPI_ANY_SOURCE, RESULT, MPI_COMM_WORLD, &status);
          #ifdef DEBUG
          printf("\nMaster received result %d from process %d",temp_result,status.MPI_SOURCE);
          fflush(stdout);
          #endif

          result += temp_result;
      }

      for(int i=1; i<nproc; i++) {
          MPI_Send(NULL, 0, MPI_INTEGER, i, FINISH, MPI_COMM_WORLD);
      }

      printf("\n\nCalculation finishied with %d prime numbers\n", result);

  } else { // SLAVE
      do {
          MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

          if(status.MPI_TAG==DATA) {
              MPI_Recv(sub_numbers, RANGE_SIZE, MPI_INTEGER, 0, DATA, MPI_COMM_WORLD, &status);
              temp_result = count_primes(sub_numbers, RANGE_SIZE);
              MPI_Send(&temp_result,1,MPI_INTEGER,0,RESULT,MPI_COMM_WORLD);
          }
      } while(status.MPI_TAG!=FINISH);
  }





  // synchronize/finalize your computations

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }
  
  MPI_Finalize();

}
