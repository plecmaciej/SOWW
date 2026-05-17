# High Performance Computing (HPC) – Laboratories

## 📌 Overview

This repository contains solutions and materials for laboratory classes in **High Performance Computing (HPC)**.

The labs focus on parallel programming using **MPI (Message Passing Interface)** and **OpenMP** and aim to introduce fundamental concepts such as:

* distributed computation
* process communication
* shared memory parallelism
* performance analysis

---

## Lab 1 – Parallel Prime Counting

The goal of this exercise was to implement a parallel program that computes the number of prime numbers in a given range.

The program takes a single input value `N` and counts how many prime numbers exist in the interval:

```
[0, N]
```

For example:

```
N = 10000000
```

---

## ⚙️ Technologies

* C
* MPI (OpenMPI)
* Makefile
* Linux environment

---

## How It Works

The program uses **parallel processing with MPI** to divide the workload among multiple processes.

### 1. Initialization

Each MPI program starts with:

```c
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Comm_size(MPI_COMM_WORLD, &nproc);
```

* `myrank` → ID of the current process
* `nproc` → total number of processes

---

### 2. Work Distribution

The computation is distributed using a **cyclic (interleaved) approach**:

```c
for (int i = myrank; i <= inputArgument; i += nproc)
```

This means:

* Process 0 computes: `0, nproc, 2*nproc, ...`
* Process 1 computes: `1, nproc+1, ...`
* etc.

This ensures:

* balanced workload
* simple implementation
* good scalability

---

### 3. Prime Number Check

Each process checks whether numbers are prime using an optimized function:

```c
bool isPrime(int i)
```

Optimizations:

* skips even numbers
* checks divisibility only up to √n
* iterates with step `2` (only odd divisors)

---

### 4. Local Computation

Each process counts its own results:

```c
long local_number = 0;

if (isPrime(i))
    local_number += 1;
```

---

### 5. Reduction (Combining Results)

All local results are combined using:

```c
MPI_Reduce(&local_number, &final_number, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
```

* `MPI_SUM` → sums results from all processes
* `0` → result is stored in process with rank 0

---

### 6. Output and Timing

Only the root process (rank 0) prints the result and execution time:

```c
if (!myrank) {
    ins__printtime(...);
    printf("the result is %ld", final_number);
}
```

---

## 🛠 Build & Run

### Build

```bash
make
```

### Run

```bash
make run <N> <MARKER>
```

Example:

```bash
make run 10000000 L1
```

---

## ⚡ Changing Number of Processes

In the `Makefile`, the number of processes is defined here:

```makefile
mpirun -np 4 ./mpi $(RUN_ARGS)
```

You can modify it, for example:

```makefile
mpirun -np 1
mpirun -np 2
mpirun -np 4
mpirun -np 8
```

---

## Lab 2 – Master-Slave Twin Prime Counting

The goal of this exercise was to implement a **master-slave (dynamic load balancing) parallel program** using MPI that computes the number of **twin prime pairs** in a given range.

The program takes a single input value `N` and counts how many twin primes exist in the interval:

```
[1, N]
```
Twin primes are pairs of prime numbers that differ by 2, for example:
```
(3, 5), (5, 7), (11, 13)
```

---

## ⚙️ Technologies

* C
* MPI (OpenMPI)
* Makefile
* Linux environment

---

## How It Works

This implementation uses a **master-slave communication model**, where:

* **Process 0 (master)** distributes work dynamically
* **Other processes (slaves)** perform computations and return results

---

### 1. Initialization

Same as in Lab 1:

```c
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Comm_size(MPI_COMM_WORLD, &nproc);
```

### 2. Task Distribution (Dynamic Scheduling)

The computation is divided into smaller chunks (ranges):
```c
#define RANGE_SIZE 1000
```

The master process sends ranges of numbers to slave processes:
```c
MPI_Send(range, 2, MPI_LONG, i, DATA, MPI_COMM_WORLD);
```

Each range is defined as:
```
[start, end]
```

Unlike Lab 1, this approach is dynamic:
* slaves request new work after finishing previous tasks
* improves load balancing
* avoids idle processes

---

### 3. Communication Protocol

The program uses message tags to distinguish communication types:
```c
#define DATA   0
#define RESULT 1
#define FINISH 2
```

* `DATA` → task assignment (range to process)
* `RESULT` → partial result from slave
* `FINISH` → signal to terminate computation

---

### 4. Slave Process Behavior

Each slave process operates in a loop:
```c
MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
```

Depending on the message:
* receives a range (`DATA`)
* computes result
* sends it back (`RESULT`)
* exits when receives `FINISH`

---

### 5. Twin Prime Computation

Each slave computes the number of twin primes in its assigned range:
```c
long FindTwinPrimes(long start, long end)
```

Key optimizations:
* skips even numbers
* uses efficient `isPrime()` check (up to √n)
* checks pairs `(i, i+2)`

---

### 6. Result Collection (Master)

The master process:
1. Sends initial tasks to all slaves
2. Receives results:
```c
MPI_Recv(&resulttemp, 1, MPI_LONG, MPI_ANY_SOURCE, RESULT, MPI_COMM_WORLD, &status);
```

3. Updates global result
4. Sends new work to the process that just finished
5. After all work is done:
   * collects remaining results
   * sends `FINISH` signal to all slaves

---

### 7. Output and Timing

Only the master process prints the result:
```c
printf("the result is %ld\n", result);
```

Execution time is also measured using:
```c
gettimeofday(...)
```

---

## 🛠 Build & Run

### Build
```bash
make
```

### Run
```bash
make run <N> <MARKER>
```

Example:
```bash
make run 10000000 L2
```

---

## ⚡ Key Differences from Lab 1

| Feature | Lab 1 (MPI Reduce) | Lab 2 (Master-Slave) |
|---|---|---|
| Work distribution | Static (cyclic) | Dynamic (on demand) |
| Communication | Collective | Point-to-point |
| Load balancing | Moderate | Improved |
| Complexity | Simple | More advanced |

---

## Lab 3 – Non-Blocking Twin Prime Counting

The goal of this exercise was to implement a **master-slave parallel program using non-blocking MPI communication** that computes the number of **twin prime pairs** in a given range.

The program takes a single input value `N` and counts how many twin primes exist in the interval:

```
[1, N]
```

Twin primes are pairs of prime numbers that differ by 2, for example:
```
(3, 5), (5, 7), (11, 13)
```

---

## ⚙️ Technologies

* C
* MPI (OpenMPI) - Non-blocking communication
* Makefile
* Linux environment

---

## How It Works

This implementation uses a **master-slave communication model with non-blocking (asynchronous) MPI operations**, where:

* **Process 0 (master)** distributes work dynamically using asynchronous sends/receives
* **Other processes (slaves)** perform computations and return results without blocking

The key difference from Lab 2 is the use of **non-blocking communication**, which enables **pipelining** – computation and communication happen simultaneously.

---

### 1. Non-Blocking Communication

Instead of blocking operations (`MPI_Send`, `MPI_Recv`):

```c
MPI_Send(range, 2, MPI_LONG, i, DATA, MPI_COMM_WORLD);       // Blocks!
MPI_Recv(&result, 1, MPI_LONG, i, RESULT, MPI_COMM_WORLD, &status);  // Blocks!
```

The program uses asynchronous operations:

```c
MPI_Isend(&sendBuf[...], 2, MPI_LONG, i, DATA, MPI_COMM_WORLD, &sendReq[i]);
MPI_Irecv(&resultTemp[i], 1, MPI_LONG, i, RESULT, MPI_COMM_WORLD, &recvReq[i]);
```

These return **immediately**, allowing the program to continue while data is being sent/received in the background.

---

### 2. Task Distribution (Master)

The master process uses arrays to track multiple concurrent operations:

```c
MPI_Request *sendReq = malloc(workers * sizeof(MPI_Request));
MPI_Request *recvReq = malloc(workers * sizeof(MPI_Request));
long *sendBuf = malloc(2 * workers * sizeof(long));
long *resultTemp = malloc(workers * sizeof(long));
```

Initial tasks are sent to all slave processes asynchronously:

```c
for (int i = 1; i < nproc; i++) {
    MPI_Isend(&sendBuf[2*(i-1)], 2, MPI_LONG, i, DATA, 
              MPI_COMM_WORLD, &sendReq[i-1]);
    MPI_Irecv(&resultTemp[i-1], 1, MPI_LONG, i, RESULT,
              MPI_COMM_WORLD, &recvReq[i-1]);
}
```

---

### 3. Waiting for Results (Master)

Instead of waiting for specific processes, the master waits for **any** result:

```c
MPI_Waitany(workers, recvReq, &completed, &status);
```

This function:
* Blocks until **any one** of the requests completes
* Returns which request finished (`completed`)
* Does not wait for all processes sequentially

This enables **pipelining**: while one slave is computing, the master can send new work to another.

---

### 4. Communication Protocol

The program uses message tags to distinguish communication types:

```c
#define DATA   0
#define RESULT 1
#define FINISH 2
```

* `DATA` → task assignment (range to process)
* `RESULT` → partial result from slave
* `FINISH` → signal to terminate computation

---

### 5. Slave Process Behavior (Non-Blocking)

Each slave process uses pipelining to overlap computation and communication:

```c
MPI_Request requests[2];
long resulttemp[2];  // Two buffers for pipelining

// Prepare to receive first task
MPI_Irecv(range, 2, MPI_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[0]);
MPI_Wait(&requests[0], &status);

while (status.MPI_TAG != FINISH) {
    // PHASE 1: Prepare asynchronously
    MPI_Irecv(nextRange, ...);              // Prepare NEXT data
    resulttemp[1] = FindTwinPrimes(...);    // Compute CURRENT (in parallel!)
    MPI_Isend(&resulttemp[0], ...);         // Send PREVIOUS (in parallel!)

    // PHASE 2: Wait for all to complete
    MPI_Waitall(2, requests, ...);

    // PHASE 3: Prepare for next iteration
    range = nextRange;
    resulttemp[0] = resulttemp[1];
}
```

This pattern enables:
* **Communication and computation to overlap**
* Reduced idle time
* Better utilization of network and CPU

---

### 6. Twin Prime Computation

Each slave computes the number of twin primes in its assigned range:

```c
long FindTwinPrimes(long start, long end)
```

Key optimizations:
* handles special cases (3, 5) and (5, 7)
* searches for pairs of form (6k-1, 6k+1)
* efficient `isPrime()` check (up to √n)

---

### 7. Result Collection (Master)

The master process:
1. Sends initial tasks to all slaves asynchronously
2. Waits for any result using `MPI_Waitany()`
3. Updates global result
4. Sends new work to the process that just finished
5. After all work is done:
   * waits for all sends to complete
   * sends `FINISH` signal to all slaves

---

### 8. Output and Timing

Only the master process prints the result:

```c
printf("Hi, I am process 0, the result is %ld\n", result);
```

Execution time is measured using:
```c
gettimeofday(...)
```

---

## 🛠 Build & Run

### Build
```bash
make
```

### Run
```bash
make run <N> <MARKER>
```

Example:
```bash
make run 10000000 L3
```

---

## ⚡ Key Differences from Lab 2

| Feature | Lab 2 (Blocking) | Lab 3 (Non-Blocking) |
|---|---|---|
| Communication | Blocking (MPI_Send/Recv) | Non-blocking (MPI_Isend/Irecv) |
| Waiting | `MPI_Recv()` on specific process | `MPI_Waitany()` on any process |
| Pipelining | Limited | Full (compute while communicating) |
| Efficiency | Good | Excellent |
| Complexity | Medium | Higher |

---

## ⚡ Key Differences from Lab 1

| Feature | Lab 1 (MPI Reduce) | Lab 3 (Non-Blocking Master-Slave) |
|---|---|---|
| Work distribution | Static (cyclic) | Dynamic (on demand) |
| Communication | Collective (MPI_Reduce) | Point-to-point non-blocking |
| Load balancing | Moderate | Excellent |
| Computation pattern | Independent | Master-slave with pipelining |
| Complexity | Simple | Advanced |

---

## Lab 4 – OpenMP Twin Prime Counting

The goal of this exercise was to implement a **parallel program using OpenMP (shared memory parallelism)** that computes the number of **twin prime pairs** in a given range.

The program takes a single input value `N` and counts how many twin primes exist in the interval:

```
[1, N]
```

Twin primes are pairs of prime numbers that differ by 2, for example:

```
(3, 5), (5, 7), (11, 13)
```

---

## ⚙️ Technologies

* C
* OpenMP
* Makefile
* Linux environment

---

## How It Works

This implementation uses **OpenMP (Open Multi-Processing)** for shared memory parallelism, where:

* **Multiple threads** work on shared data within a single process
* No message passing required (unlike MPI)
* Simpler programming model than distributed memory approaches
* Suitable for multi-core systems within a single machine

---

### 1. Initialization

The program sets the number of threads using:

```c
omp_set_num_threads(ins__args.n_thr);
```

This sets the number of parallel threads that will be used during execution.

---

### 2. Initialization of Twin Primes Count

To handle the special case of the first twin prime pair (3, 5):

```c
long total_twin_primes = 1;
```

The counter is initialized to 1, accounting for the pair (3, 5) which is already counted before the parallel loop begins.

---

### 3. Optimized Twin Prime Checking

The algorithm uses an efficient optimization based on the mathematical property that all twin primes (except 3, 5) follow the pattern:

```c
for( i = 6; i < inputArgument; i += 6 )
```

This means:
* Process pairs at positions of form `6k-1` and `6k+1`
* First iteration checks: `(5, 7), (11, 13), (17, 19), ...`
* Second iteration checks: `(11, 13), (17, 19), (23, 25), ...`
* etc.

Why this works:
* All integers can be expressed as `6k`, `6k+1`, `6k+2`, `6k+3`, `6k+4`, or `6k+5`
* Only `6k-1` and `6k+1` can both be prime (other forms are always divisible by 2 or 3)
* Reduces search space by a factor of 6 compared to checking all numbers

---

### 4. Prime Number Check

Each thread checks whether numbers are prime using an optimized function:

```c
bool isPrime(int i)
```

Optimizations:

* skips even numbers
* checks divisibility only up to √n
* iterates with step `2` (only odd divisors)
* handles special cases (2 and 3 implicitly)

---

### 5. Parallel Loop with OpenMP

The main computation is parallelized using a parallel for loop:

```c
#pragma omp parallel for private(result) reduction(+:total_twin_primes)
for( i = 6; i < inputArgument; i += 6 ) {
    result = 0;
    if (isPrime(i-1) && isPrime(i + 1)) {
        result++;
    }
    total_twin_primes = total_twin_primes + result;
}
```

Key directives:

* `#pragma omp parallel for` → parallelize the loop across available threads
* `private(result)` → each thread has its own copy of `result` variable
* `reduction(+:total_twin_primes)` → safely accumulate results from all threads using addition

How it works:

* The loop iterations are distributed among threads (static or dynamic scheduling)
* Each thread independently checks pairs for primality
* Results are combined using a thread-safe reduction operation
* No data race conditions occur due to the `reduction` clause

---

## 🛠 Build & Run

### Build

```bash
make
```

### Run

```bash
make run <N> <MARKER> <NUM_THREADS>
```

Example:

```bash
make run 10000000 L4 4
```

---

## ⚡ Changing Number of Threads

The number of threads can be specified via command-line argument:

```bash
./openmp --arg 10000000 --marker L4 --n_thr 1
./openmp --arg 10000000 --marker L4 --n_thr 2
./openmp --arg 10000000 --marker L4 --n_thr 4
./openmp --arg 10000000 --marker L4 --n_thr 8
```

Or via the `OMP_NUM_THREADS` environment variable:

```bash
export OMP_NUM_THREADS=4
./openmp --arg 10000000 --marker L4
```

---

## ⚡ Key Differences from Previous Labs

| Feature | Lab 1 (MPI Reduce) | Lab 2 (MPI Master-Slave) | Lab 3 (MPI Non-Blocking) | Lab 4 (OpenMP) |
|---|---|---|---|---|
| Parallelism model | Distributed memory | Distributed memory | Distributed memory | Shared memory |
| Communication | Collective reduction | Point-to-point blocking | Point-to-point non-blocking | None (shared variables) |
| Work distribution | Static (cyclic) | Dynamic (on demand) | Dynamic with pipelining | Static or dynamic scheduling |
| Communication overhead | Low (MPI_Reduce) | Moderate (frequent messaging) | Low (pipelined) | Very low (shared memory) |
| Ease of use | Medium | Medium | Complex | Simple |
| Scalability (single machine) | Limited | Limited | Limited | Excellent |

---

## Lab 5 – Hybrid MPI + OpenMP Twin Prime Counting

The goal of this exercise was to implement a **hybrid parallel program combining both MPI (distributed memory) and OpenMP (shared memory)** that computes the number of **twin prime pairs** in a given range.

The program takes a single input value `N` and counts how many twin primes exist in the interval:

```
[1, N]
```

Twin primes are pairs of prime numbers that differ by 2, for example:

```
(3, 5), (5, 7), (11, 13)
```

---

## ⚙️ Technologies

* C
* MPI (OpenMPI)
* OpenMP
* Makefile
* Linux environment

---

## How It Works

This implementation combines **MPI for inter-process communication** and **OpenMP for intra-process parallelism**, where:

* **Each MPI process** controls multiple OpenMP threads
* **MPI processes** are distributed across machines or cores
* **OpenMP threads** within each process work on shared memory
* Data is partitioned among MPI processes, then each process uses OpenMP to parallelize its portion

This is a **hybrid approach** that leverages both distributed and shared memory parallelism.

---

### 1. MPI Initialization with Threading Support

The program initializes MPI with thread support:

```c
int threadsupport;
MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &threadsupport);

if (threadsupport < MPI_THREAD_FUNNELED) {
    printf("\nThe implementation does not support MPI_THREAD_FUNNELED, it supports level %d\n", threadsupport);
    MPI_Finalize();
    return -1;
}
```

Key points:

* `MPI_Init_thread()` instead of `MPI_Init()` allows threading
* `MPI_THREAD_FUNNELED` level means only the main thread calls MPI functions
* Verification ensures the system supports the required threading level

---

### 2. OpenMP Thread Setup

The number of OpenMP threads is set:

```c
omp_set_num_threads(ins__args.n_thr);
```

This determines how many threads each MPI process will spawn.

---

### 3. MPI Process and Rank Information

Standard MPI initialization:

```c
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Comm_size(MPI_COMM_WORLD, &nproc);
```

* `myrank` → unique ID of current MPI process
* `nproc` → total number of MPI processes

---

### 4. Work Distribution Between MPI Processes

The work is statically divided among MPI processes:

```c
step = (double) inputArgument / nproc;
calculate(myrank);
```

Each process gets a range `[start, end]`:

* Process 0: `[0, step)`
* Process 1: `[step, 2*step)`
* Process 2: `[2*step, 3*step)`
* etc.

---

### 5. Calculate Function with OpenMP Parallelization

Each MPI process executes its own portion using OpenMP parallelism:

```c
void calculate(int rank) {
    long result;
    long total_result = 0;
    int start = rank * step;
    
    // Align start to 6k pattern for optimization
    if (start % 6 != 0)
        start = start + (6 - start % 6);

    int end = rank * step + step;
    int i = 0;

    #pragma omp parallel for private(result) reduction(+:total_result)
    for(i = start; i < end; i += 6) {
        result = 0;

        if (isPrime(i-1) && isPrime(i + 1)) {
            result++;
        }

        total_result = total_result + result;
    }

    MPI_Send(&total_result, 1, MPI_LONG, 0, RESULT, MPI_COMM_WORLD);
}
```

Key features:

* **Range alignment**: Each process adjusts its start to the `6k` pattern
* **OpenMP parallelization**: The loop `for(i = start; i < end; i += 6)` is parallelized
* **Thread-safe reduction**: `reduction(+:total_result)` safely combines partial results from all threads
* **Result sending**: Each process sends its result back to rank 0 using MPI

---

### 6. Optimized Twin Prime Checking (6k±1 Pattern)

As in Lab 4, the algorithm exploits the mathematical property:

```c
for(i = start; i < end; i += 6)
```

All twin primes (except 3, 5) follow the form `(6k-1, 6k+1)`. This reduces the search space by a factor of 6.

---

### 7. Prime Number Check

Each thread checks primality:

```c
bool isPrime(int i) {
    long sqrttt = sqrt(i);
    for (int j = 5; j <= sqrttt; j += 2) {
        if (i % j == 0) return false;
    }
    return true;
}
```

Optimizations:
* Checks divisibility only up to √n
* Iterates with step 2 (only odd divisors)

---

### 8. Result Collection (Master Process)

Only process 0 collects results from all other processes:

```c
if (!myrank) {
    long resulttemp;
    for (i = 0; i < nproc; i++) {
        MPI_Recv(&resulttemp, 1, MPI_LONG, i, RESULT, MPI_COMM_WORLD, &status);
        printf("\nReceived result %ld for process %d\n", resulttemp, i);
        fflush(stdout);
        result_final += resulttemp;
    }

    printf("\nHi, I am process 0, the result is %ld\n", result_final);
}
```

Process 0:
* Receives results from all processes (including itself)
* Accumulates partial results
* Prints the final total

---

### 9. Output and Timing

Timing is measured only on process 0:

```c
if(!myrank) {
    gettimeofday(&ins__tstart, NULL);
}

// ... computation ...

if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
}
```

---

## 🛠 Build & Run

### Build

```bash
make
```

### Run Locally (Single Machine)

```bash
make run <N> <MARKER> <NUM_THREADS> <NUM_PROCESSES>
```

Example:

```bash
mpirun -np 2 ./openmp+mpi --arg 10000000 --marker L5 --n_thr 4
```

This runs 2 MPI processes, each with 4 OpenMP threads (8 threads total).

---

### Run on Distributed Machines (Cluster)

To run on multiple machines in a lab environment, first create a `machinefile` containing the hostnames:

```
machine1.lab.com
machine2.lab.com
machine3.lab.com
```

Then SSH into each machine, compile the code, and run:

```bash
mpirun -machinefile ./machinefile --mca orte_keep_fqdn_hostnames t --mca btl_tcp_if_exclude docker0,docker_gwbridge,lo -np 3 ./openmp+mpi --arg 1000000 --marker L5 --n_thr 4
```

**Explanation of flags:**

* `-machinefile ./machinefile` → Use the specified machine file with host list
* `--mca orte_keep_fqdn_hostnames t` → Keep fully qualified domain names
* `--mca btl_tcp_if_exclude docker0,docker_gwbridge,lo` → Exclude docker and loopback interfaces from TCP communication
* `-np 3` → Run 3 MPI processes (one per machine in this example)
* `./openmp+mpi --arg 1000000 --marker L5 --n_thr 4` → Run the program with 1M input, label L5, and 4 threads per process

**Setup steps for cluster deployment:**

1. Ensure all machines are accessible via SSH
2. Compile the code on your development machine
3. Copy the compiled binary to all machines in the cluster:
   ```bash
   scp ./openmp+mpi user@machine1.lab.com:~/
   scp ./openmp+mpi user@machine2.lab.com:~/
   scp ./openmp+mpi user@machine3.lab.com:~/
   ```
4. Create and distribute the `machinefile`
5. Run the mpirun command from any of the machines

---

## ⚡ Key Differences from Previous Labs

| Feature | Lab 1 (MPI Reduce) | Lab 4 (OpenMP) | Lab 5 (Hybrid MPI+OpenMP) |
|---|---|---|---|
| Parallelism model | Distributed memory | Shared memory | Hybrid (distributed + shared) |
| Communication | Collective reduction | None (shared variables) | Point-to-point MPI |
| Process count | Multiple | Single (threads only) | Multiple with multiple threads each |
| Work distribution | Static (cyclic) | Static (loop scheduling) | Static (MPI), dynamic (OpenMP) |
| Scalability (cluster) | Good | N/A | Excellent |
| Scalability (single machine) | Limited | Excellent | Good |

---

## 🎓 Key Concepts

### MPI_THREAD_FUNNELED vs Other Threading Levels

* `MPI_THREAD_SINGLE` → Only one thread uses MPI (most restrictive)
* `MPI_THREAD_FUNNELED` → Only the main thread calls MPI functions
* `MPI_THREAD_SERIALIZED` → Multiple threads call MPI, but not concurrently
* `MPI_THREAD_MULTIPLE` → Full thread safety (least common, highest overhead)


