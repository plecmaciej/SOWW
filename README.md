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
