#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include "numgen.c"
#include "math.h"



int checkIfPrime(long value){

  int flag = 1; 
  
  if(value == 0 || value == 1)
    return 0;

    double sqroot = sqrt(value); 
  
    // Iterate from 2 to sqrt(n) 
    for (int i = 2; i <= sqroot; i++) { 
  
        // If n is divisible by any number between 2 and 
        // sqrt(n), it is not prime 
        if (value % i == 0) { 
            flag = 0; 
            break; 
        } 
    } 

  if (flag)
    printf("%lu\n",value);
  return flag;

}

int main(int argc,char **argv) {


  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //set number of threads
  omp_set_num_threads(ins__args.n_thr);
  
  //program input argument
  long inputArgument = ins__args.arg; 
  unsigned long int *numbers = (unsigned long int*)malloc(inputArgument * sizeof(unsigned long int));
  numgen(inputArgument, numbers);

  struct timeval ins__tstart, ins__tstop;
  gettimeofday(&ins__tstart, NULL);

  int i = 0;
  long sum = 0;
  long val = 0;
  // run your computations here (including OpenMP stuff)
  #pragma omp parallel for default(none) shared(numbers, inputArgument) private(val) reduction(+:sum)
  for (i = 0; i < inputArgument; i++)
  {
    val = checkIfPrime(numbers[i]);
    sum += val;
  }

  // #pragma omp parallel for default(none) shared(numbers, inputArgument, sum) private(val)
  // for (i = 0; i < inputArgument; i++)
  // {
  //   val = checkIfPrime(numbers[i]);
  //   if(val != 0)
  //   #pragma omp atomic update
  //     sum++;
  // }

      printf("sum: %lu\n",sum);
  
  
  // synchronize/finalize your computations
  gettimeofday(&ins__tstop, NULL);
  ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);

}
