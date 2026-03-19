#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>

bool isPrime(int i){
  if (i < 2) return false;
  else if (i == 2) return true;
  else if (i % 2 == 0) return false;
  else{
    long sqrttt =  sqrt(i);
    for(int j = 3; j <= sqrttt; j+=2){
      if (i % j == 0) return false;
    }

  }
  return true;

}

int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;

  int myrank,nproc;
  
  MPI_Init(&argc,&argv);

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if(!myrank)
      gettimeofday(&ins__tstart, NULL);


  // run your computations here (including MPI communication)

  long local_number = 0;
  for (int i = myrank; i<=inputArgument; i+=nproc){
    if (isPrime(i))local_number += 1;
  }

  long final_number;

  MPI_Reduce (&local_number, &final_number, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  // synchronize/finalize your computations

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
    printf("the result is %ld", final_number);
  }
  
  MPI_Finalize();

}
