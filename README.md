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
