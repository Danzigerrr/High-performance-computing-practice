#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;

  int myrank,nproc;
  int check_prime_final;
  
  MPI_Init(&argc,&argv);

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if(!myrank)
      gettimeofday(&ins__tstart, NULL);


  // run your computations here (including MPI communication)

  int check_prime = 0;
  long i = myrank;

  // printf("myrank: %d\n", myrank);
  // printf("inputArgument: %ld\n", inputArgument);

  while(i < inputArgument){
    // printf("myrank: %d my int: %d\n", myrank, i);

    // corner cases
    // if (i == 0 || i == 1) {
    //      check_prime++;
    // }

    if(i > 1){
      int remainder = inputArgument % i;
      if(remainder == 0){
        // printf("can be didved by: %d\n", i);
        check_prime++;
        break;
      }
    }
    i = i + nproc;
  }

  MPI_Reduce(&check_prime,&check_prime_final,1,
  MPI_INT,MPI_SUM,0,
  MPI_COMM_WORLD);

  if(!myrank){
    if(check_prime_final != 0)
      printf("\nis not prime\n");
    else
      printf("\nis prime\n");
  }


  // synchronize/finalize your computations

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }

  MPI_Finalize();
  return 0;
}
