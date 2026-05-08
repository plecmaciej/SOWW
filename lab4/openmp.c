#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include "numgen.c"
#include <math.h>
#include <stdbool.h>


bool isPrime(int i) {
    long sqrttt = sqrt(i);
    for (int j = 5; j <= sqrttt; j += 2) {
        if (i % j == 0) return false;
    }
    
    return true;
}

int main(int argc,char **argv) {


  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //set number of threads
  omp_set_num_threads(ins__args.n_thr);
  
  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;
  gettimeofday(&ins__tstart, NULL);
  
  // run your computations here (including OpenMP stuff)

  long i=0;
  long result;
  // we add initially the pair of (3,5)
  long total_twin_primes = 1;

  #pragma omp parallel for private(result) reduction(+:total_twin_primes)
  for( i = 6; i < inputArgument; i += 6 ) {
    result = 0;

    if (isPrime(i-1) && isPrime(i + 1)) {
        result++;
    }

    total_twin_primes = total_twin_primes + result;
  }


  printf("\nHi, the total twin primes result is %ld\n", total_twin_primes);
  
  // synchronize/finalize your computations
  gettimeofday(&ins__tstop, NULL);
  ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);

}
