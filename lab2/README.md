# Prime Number Calculation with MPI

This program calculates prime numbers using the Message Passing Interface (MPI) framework in C. It distributes the workload among multiple processes for parallel computation.

## Prerequisites

- **MPI Implementation:** Ensure that you have an MPI implementation installed on your system, such as Open MPI or MPICH.

## Compilation

Compile the code using the provided Makefile. Run the following command in the terminal:

```bash
make
```

This will generate the executable `mpi`.

## Usage

### Running the Program

To execute the program, use the following command:

```bash
make run <inputArgument>
```

Replace `<inputArgument>` with the desired upper limit for prime number calculation. For example:

```bash
make run 1000000
```

This command will calculate prime numbers up to 1,000,000 using 8 MPI processes.

### Cleaning Up

To clean up generated files and executables, use the following command:

```bash
make clean
```

This will remove the `mpi` executable.

## Notes

- The program distributes the workload evenly among MPI processes for efficient parallel computation.
- Make sure to adjust the number of MPI processes based on your system configuration and available resources.
- The program provides debug information if the `DEBUG` flag is defined. You can toggle this in the source code (`mpi.c`) as needed.

---
