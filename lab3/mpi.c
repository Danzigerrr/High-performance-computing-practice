#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <sys/time.h>
#include <mpi.h>
#include "numgen.c"

#define PACKAGE_SIZE 2
#define DATA 0
#define RESULT 1
#define FINISH 2

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
  //program input argument
  long values_counter = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;

  int myrank, nproc;
  int requestcompleted;
  int requestcount = 0;
  int sentcount = 0;
  int recvcount = 0;
  
  unsigned long int *numbers;
  unsigned long int packet[PACKAGE_SIZE];
  MPI_Status status;
  MPI_Request *requests;

  int *resulttemp;

  MPI_Init(&argc,&argv);

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if (nproc < 2)
  {
    printf("Run with at least 2 processes");
    MPI_Finalize();
    return -1;
  }

  if(!myrank){
    int additional_values = (PACKAGE_SIZE - values_counter % PACKAGE_SIZE )%PACKAGE_SIZE;

  // generate numbers
    gettimeofday(&ins__tstart, NULL);
	  numbers = (unsigned long int*)malloc((values_counter + additional_values) * sizeof(unsigned long int));
  	numgen(values_counter, numbers);

  // fill the array with additional non-prime values
    for(int i=0;i<additional_values;i++){
      numbers[values_counter+i] = 0;
    }
    values_counter += additional_values;
  }

   for (unsigned long int i = 0; i < values_counter; i++) //check nubmers
	  {
      numbers[i] = i;
          // printf("%lu\n", numbers[i]);
    }


  // run your computations here (including MPI communication)
  if (myrank == 0) // Master
  {
    requests = (MPI_Request *) malloc (3 * (nproc - 1) * sizeof (MPI_Request));
    resulttemp = (int *) malloc ((nproc - 1) * sizeof (int));

    //printf("Created Master");
    int full_prime_count = 0;
    //int resulttemp = 0;
    int sent_index = 0;
    int array_size = values_counter;
    for (int i = 1; i < nproc; i++) //First send
	  {
      if(sent_index < array_size){
        // printf("value:%d\n", numbers+sent_index);
        MPI_Send(numbers+sent_index, PACKAGE_SIZE, MPI_UNSIGNED_LONG, i, DATA, MPI_COMM_WORLD);
        sent_index += PACKAGE_SIZE;
      }
    }

    // Initialize MPI requests for asynchronous communication
    for (int i = 0; i < 2 * (nproc - 1); i++)
  		requests[i] = MPI_REQUEST_NULL;	// no active at this point

    // Start receiving results from slave processes asynchronously    
    for (int i = 1; i < nproc; i++) 
      MPI_Irecv(&(resulttemp[i - 1]), 1, MPI_INT, i, RESULT, MPI_COMM_WORLD, &(requests[i - 1]));

    // Continue sending more work to slave processes
    for (int i = 1; i < nproc;i++){
      MPI_Isend (numbers+sent_index, PACKAGE_SIZE, MPI_UNSIGNED_LONG, i, DATA, MPI_COMM_WORLD,&(requests[nproc - 2 + i]));
      sent_index += PACKAGE_SIZE;
    }

    // Loop until all numbers are processed
    while(sent_index < array_size){
      // Wait for any completed request
	    MPI_Waitany(2 * nproc - 2, requests, &requestcompleted, MPI_STATUS_IGNORE);
    // printf("requestcompleted: %d", requestcompleted);

      if (requestcompleted < (nproc - 1))
	    {
        full_prime_count += resulttemp[requestcompleted]; //save the result
		    MPI_Wait(&(requests[nproc - 1 + requestcompleted]),MPI_STATUS_IGNORE); // wait for the send message to complete
        
        MPI_Isend(numbers+sent_index, PACKAGE_SIZE, MPI_UNSIGNED_LONG, requestcompleted + 1, DATA, MPI_COMM_WORLD, &(requests[nproc - 1 + requestcompleted])); // send more data to slave
        
        sent_index += PACKAGE_SIZE;
        MPI_Irecv(&(resulttemp[requestcompleted]), 1, MPI_INT, requestcompleted + 1, RESULT, MPI_COMM_WORLD, &(requests[requestcompleted])); //set up the next Irecv and its buffer
      }
    }

    // Send finish signal to all slave processes
    for (int i = 1; i < nproc; i++)
    {
      MPI_Isend (NULL, 0, MPI_UNSIGNED_LONG, i, FINISH, MPI_COMM_WORLD, &(requests[2 * nproc - 3 + i]));
    }

    // Wait for all pending communication to finish
    MPI_Waitall (3 * nproc - 3, requests, MPI_STATUSES_IGNORE);
    
    // Add up the final result from each slave process   
    for (int i = 0; i < (nproc - 1); i++)
    {
      full_prime_count += resulttemp[i];
    }

    // receive synchronously and update the count
    for (int i = 0; i < (nproc - 1); i++)
    {
      MPI_Recv (&(resulttemp[i]), 1, MPI_INT, i+1, RESULT, MPI_COMM_WORLD, &status);
      full_prime_count += resulttemp[i];
    }

    printf ("\nFound %d Prime Numbers\n", full_prime_count);
  }




  else //Slave
  {
    requests = (MPI_Request *) malloc (2 * sizeof (MPI_Request));
    requests[0] = requests[1] = MPI_REQUEST_NULL;
    numbers = (unsigned long int*)malloc(PACKAGE_SIZE * sizeof(unsigned long int));
    // resulttemp = (int *) malloc (2 * sizeof (int));
    MPI_Status statuses[2];

    // receive first package from master
    MPI_Recv(packet, PACKAGE_SIZE, MPI_UNSIGNED_LONG , 0, DATA, MPI_COMM_WORLD, &status);

    do
    {    
      // receive numbers from slave if there are any            
        MPI_Irecv (numbers, PACKAGE_SIZE, MPI_UNSIGNED_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &(requests[0]));

        int prime_count = 0;
        for(int i = 0; i < PACKAGE_SIZE; i++)
        {
          prime_count += checkIfPrime(packet[i]);
        }
        // resulttemp[1] = prime_count;

        // Wait for the completion of send and receive operations initiated previously
        MPI_Waitall (2, requests, &statuses);

        if (statuses[0].MPI_TAG == DATA){
          for(int i = 0; i < PACKAGE_SIZE; i++)
            packet[i] = numbers[i];
        }
        // resulttemp[0] = resulttemp[1];

        // send the result back to master
        MPI_Isend (&prime_count, 1, MPI_INT, 0, RESULT, MPI_COMM_WORLD, &(requests[1]));
        
  	} while (statuses[0].MPI_TAG != FINISH);


    MPI_Wait(&(requests[1]), MPI_STATUS_IGNORE);
  }

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }
  
  MPI_Finalize();
}
