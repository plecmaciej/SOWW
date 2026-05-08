#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>
#include "numgen.c"
#include <math.h>
#include <stdbool.h>

#define RESULT 1

double step;
int myrank, proccount;

bool isPrime(int i) {
    long sqrttt = sqrt(i);
    for (int j = 5; j <= sqrttt; j += 2) {
        if (i % j == 0) return false;
    }
    
    return true;
}


void calculate (int rank)
{
    long result;
    long total_result = 0;
    int start = rank*step;
    if ( start % 6 != 0 )
        start = start + (6 - start % 6);

    int end = rank*step+step;
    int i=0;

    #pragma omp parallel for private(result) reduction(+:total_result)
    for( i = start; i < end; i+=6) {
        result = 0;

        if (isPrime(i-1) && isPrime(i + 1)) {
            result++;
        }

        total_result = total_result + result;
    }

    MPI_Send (&total_result, 1, MPI_LONG, 0, RESULT, MPI_COMM_WORLD);
}

int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //set number of threads
  omp_set_num_threads(ins__args.n_thr);

  MPI_Status status;
  
  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;

  int threadsupport;
  int myrank,nproc;
  unsigned long int *numbers;
  // Initialize MPI with desired support for multithreading -- state your desired support level

  MPI_Init_thread(&argc, &argv,MPI_THREAD_FUNNELED,&threadsupport); 

  if (threadsupport<MPI_THREAD_FUNNELED) {
    printf("\nThe implementation does not support MPI_THREAD_FUNNELED, it supports level %d\n",threadsupport);
    MPI_Finalize();
    return -1;
  }
  
  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if(!myrank){
      gettimeofday(&ins__tstart, NULL);
  }
  // run your computations here (including MPI communication and OpenMP stuff)
  long result_final = 0;
  int i;

  step = (double) inputArgument / nproc;
  calculate(myrank);

  if (!myrank)
  {
      // receive results from the threads
    long resulttemp;
    for (i = 0; i < nproc; i++)
      {
          MPI_Recv (&resulttemp, 1, MPI_LONG, i, RESULT, MPI_COMM_WORLD, &status);
          printf ("\nReceived result %ld for process %d\n",resulttemp, i);
          fflush (stdout);
          result_final += resulttemp;
    }

    printf("\nHi, I am process 0, the result is %ld\n", result_final);
  }

  // synchronize/finalize your computations

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }
    
  MPI_Finalize();
  
}
