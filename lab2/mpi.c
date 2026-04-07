#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include "numgen.c"
#include <math.h>
#include <stdbool.h>

#define RANGE_SIZE 1000
#define DATA 0
#define RESULT 1
#define FINISH 2

bool isPrime(int i) {
	if (i < 2) return false;
	else if (i == 2) return true;
	else if (i % 2 == 0) return false;
	else {
		long sqrttt = sqrt(i);
		for (int j = 3; j <= sqrttt; j += 2) {
			if (i % j == 0) return false;
		}

	}
	return true;

}

long FindTwinPrimes(long start, long end) {
	long result = 0;
	long begin = start;

	if(start == 2){
		result += 1;
		start = 3;
	}
	
	if (start % 2 == 1){

		begin = start + 1;
	}


	for (long i = begin; i <= end; i+=2) {
		if (isPrime(i) && isPrime(i + 2)) {
			result++;
		}
	
	}

	return result;
}

int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;

  int myrank,nproc;
  unsigned long int *numbers;

  MPI_Init(&argc,&argv);

  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if(!myrank){
      	gettimeofday(&ins__tstart, NULL);
  }

  long range[2];
  int sentcount = 0;
  long result = 0, resulttemp;
  MPI_Status status;

  if (myrank == 0) {
	  range[0] = 1;

	  for (int i = 1; i < nproc; i++)
	  {
		  range[1] = range[0] + RANGE_SIZE;
		  MPI_Send(range, 2, MPI_LONG, i, DATA, MPI_COMM_WORLD);
		  sentcount++;
		  range[0] = range[1] + 1;
	  }

	  do
	  {
		  MPI_Recv(&resulttemp, 1, MPI_LONG, MPI_ANY_SOURCE, RESULT, MPI_COMM_WORLD, &status);

		  result += resulttemp;
		  range[1] = range[0] + RANGE_SIZE;

		  if (range[1] > inputArgument) range[1] = inputArgument;

		  MPI_Send(range, 2, MPI_LONG, status.MPI_SOURCE, DATA, MPI_COMM_WORLD);

		  range[0] = range[1] + 1;

	  } while (range[1] < inputArgument);

	  for (int i = 0; i < (nproc - 1); i++)
	  {
		  MPI_Recv(&resulttemp, 1, MPI_LONG, MPI_ANY_SOURCE, RESULT, MPI_COMM_WORLD, &status);

		  result += resulttemp;
	  }

	  for (int i = 1; i < nproc; i++)
	  {
		  MPI_Send(NULL, 0, MPI_LONG, i, FINISH, MPI_COMM_WORLD);
	  }

	  printf("\nHi, I am process 0, the result is %ld\n", result);
  }
  else {			
		do
		{
			MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if (status.MPI_TAG == DATA)
			{
				MPI_Recv(range, 2, MPI_LONG, 0, DATA, MPI_COMM_WORLD, &status);

				resulttemp = FindTwinPrimes(range[0], range[1]);

				MPI_Send(&resulttemp, 1, MPI_LONG, 0, RESULT, MPI_COMM_WORLD);
			}

		} while (status.MPI_TAG != FINISH);
  }



  // synchronize/finalize your computations

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }
  
  MPI_Finalize();

}
