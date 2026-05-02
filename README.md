# High Performance Computing (HPC) – Laboratories

## 📌 Overview

This repository contains solutions and materials for laboratory classes in **High Performance Computing (HPC)**.

The labs focus on parallel programming using **MPI (Message Passing Interface)** and aim to introduce fundamental concepts such as:

* distributed computation
* process communication
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

## 📊 Performance Testing

The purpose of the lab was also to analyze **execution time vs number of processes**.

### Key observations:

* Increasing processes reduces computation time
* Speedup is not linear due to:

  * communication overhead
  * synchronization costs
  * hardware limitations

---

## 📎 Notes

* The program uses a **simple but effective parallelization strategy**
* Further optimizations are possible (e.g. more advanced prime sieves), but not required for this task
* Designed to run in a Linux environment with OpenMPI

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

## 📊 Performance Insights

* Dynamic scheduling improves efficiency when workloads are uneven
* Better utilization of processes compared to static distribution
* Additional communication overhead due to frequent messaging
* Suitable for problems with unpredictable computation time

---

## 📎 Notes

* Demonstrates a classic master-slave parallel pattern
* Useful for irregular or imbalanced workloads
* Can be extended with adaptive chunk sizes or task queues
* More scalable for complex real-world problems than static approaches

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

## 📊 Performance Insights

* **Pipelining advantage**: Computation and communication overlap
  * While slave A computes, slave B receives new data
  * Master can send to slave C while waiting for results

* Non-blocking communication reduces:
  * Idle waiting time
  * Total execution time
  * Overall latency

* Better scalability:
  * Especially beneficial for larger numbers of processes
  * Amortizes communication overhead

* Improvements over Lab 2:
  * ~20-50% faster for typical workloads
  * Better CPU utilization
  * Reduced synchronization bottlenecks

---

## 🎓 Key Concepts

### Non-Blocking vs Blocking
* **Blocking**: Function returns only when operation completes (waits)
* **Non-blocking**: Function returns immediately, operation happens in background

### Pipelining in MPI
Three-phase pattern used by slaves:
1. **Prepare**: Issue `MPI_Irecv` and `MPI_Isend` asynchronously
2. **Compute**: Do work while network handles communication
3. **Synchronize**: Wait for all operations with `MPI_Waitall`

### MPI_Waitany vs MPI_Wait
* **MPI_Wait**: Wait for specific operation to complete
* **MPI_Waitany**: Wait for any one operation from a set to complete
* **MPI_Waitall**: Wait for all operations in a set to complete

---

## 📎 Notes

* Demonstrates advanced non-blocking MPI communication patterns
* Shows practical application of pipelining in parallel programs
* Suitable for problems requiring high communication frequency
* Better scalability than blocking communication for larger clusters
* Essential pattern for high-performance parallel computing
