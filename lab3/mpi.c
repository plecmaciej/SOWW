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


    if (start < 10 ) {
        result += 3;
        start = 12;
    }

    long begin = start + 6 - (start % 6);


    for (long i = begin; i <= end; i += 6) {
        if (isPrime(i-1) && isPrime(i + 1)) {
            result++;
        }
    }

    return result;
}

int main(int argc, char **argv) {

    Args ins__args;
    parseArgs(&ins__args, &argc, argv);

    long inputArgument = ins__args.arg;

    struct timeval ins__tstart, ins__tstop;

    int myrank, nproc;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (!myrank) {
        gettimeofday(&ins__tstart, NULL);
    }

    if (nproc < 2) {
        if (!myrank) {
            printf("Run with at least 2 processes\n");
        }
        MPI_Finalize();
        return 0;
    }

    long result = 0;
    MPI_Status status;

    if (myrank == 0) {
        int workers = nproc - 1;

        MPI_Request *sendReq = (MPI_Request *)malloc(workers * sizeof(MPI_Request));
        MPI_Request *recvReq = (MPI_Request *)malloc(workers * sizeof(MPI_Request));
        long *sendBuf = (long *)malloc(2 * workers * sizeof(long));
        long *resultTemp = (long *)malloc(workers * sizeof(long));

        if (!sendReq || !recvReq || !sendBuf || !resultTemp) {
            printf("Not enough memory\n");
            MPI_Finalize();
            return -1;
        }

        for (int i = 0; i < workers; i++) {
            sendReq[i] = MPI_REQUEST_NULL;
            recvReq[i] = MPI_REQUEST_NULL;
        }

        long nextStart = 1;
        int activeWorkers = 0;

        // begining of sending 

        for (int i = 1; i < nproc; i++) {
            if (nextStart <= inputArgument) {
                long nextEnd = nextStart + RANGE_SIZE;
                if (nextEnd > inputArgument) nextEnd = inputArgument;

                sendBuf[2 * (i - 1)] = nextStart;
                sendBuf[2 * (i - 1) + 1] = nextEnd;

                MPI_Isend(&sendBuf[2 * (i - 1)], 2, MPI_LONG, i, DATA,
                          MPI_COMM_WORLD, &sendReq[i - 1]);

                MPI_Irecv(&resultTemp[i - 1], 1, MPI_LONG, i, RESULT,
                          MPI_COMM_WORLD, &recvReq[i - 1]);

                activeWorkers++;
                nextStart = nextEnd + 1;
            } else {
                MPI_Isend(NULL, 0, MPI_LONG, i, FINISH, MPI_COMM_WORLD, &sendReq[i - 1]);
            }
        }

        //waiting for any result

        while (activeWorkers > 0) {
            int completed;
            MPI_Waitany(workers, recvReq, &completed, &status);

            if (completed == MPI_UNDEFINED) {
                break;
            }

            result += resultTemp[completed];

            // is send completed 

            MPI_Wait(&sendReq[completed], MPI_STATUS_IGNORE);

            if (nextStart <= inputArgument) {
                long nextEnd = nextStart + RANGE_SIZE;
                if (nextEnd > inputArgument) nextEnd = inputArgument;

                sendBuf[2 * completed] = nextStart;
                sendBuf[2 * completed + 1] = nextEnd;

                MPI_Isend(&sendBuf[2 * completed], 2, MPI_LONG,
                          completed + 1, DATA, MPI_COMM_WORLD,
                          &sendReq[completed]);

                MPI_Irecv(&resultTemp[completed], 1, MPI_LONG,
                          completed + 1, RESULT, MPI_COMM_WORLD,
                          &recvReq[completed]);

                nextStart = nextEnd + 1;
            } else {

                MPI_Isend(NULL, 0, MPI_LONG, completed + 1, FINISH,
                          MPI_COMM_WORLD, &sendReq[completed]);

                recvReq[completed] = MPI_REQUEST_NULL;
                activeWorkers--;
            }
        }

     

        MPI_Waitall(workers, sendReq, MPI_STATUSES_IGNORE);

        printf("\nHi, I am process 0, the result is %ld\n", result);

        free(sendReq);
        free(recvReq);
        free(sendBuf);
        free(resultTemp);
    } else {

        // revieve and prepere to send 

        MPI_Request requests[2];
        long range[2];
        long nextRange[2];
        long resulttemp;

        requests[0] = MPI_REQUEST_NULL;
        requests[1] = MPI_REQUEST_NULL;

        MPI_Irecv(range, 2, MPI_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[0]);
        MPI_Wait(&requests[0], &status);

        while (status.MPI_TAG != FINISH) {


            // master can send another
            MPI_Irecv(nextRange, 2, MPI_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[0]);
            resulttemp = FindTwinPrimes(range[0], range[1]);

            MPI_Isend(&resulttemp, 1, MPI_LONG, 0, RESULT, MPI_COMM_WORLD, &requests[1]);


            MPI_Wait(&requests[0], &status);
            range[0] = nextRange[0];
            range[1] = nextRange[1];


            if (status.MPI_TAG == FINISH) {
                break;
            }

            MPI_Wait(&requests[1], MPI_STATUS_IGNORE);
            

        }
    }

    if (!myrank) {
        gettimeofday(&ins__tstop, NULL);
        ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
    }

    MPI_Finalize();
    return 0;
}