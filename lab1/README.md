**MPI Prime Number Checker**

This program is designed to check whether a given number is prime using MPI (Message Passing Interface) for parallel processing. It distributes the workload of checking divisors across multiple processes to improve efficiency.

### How to Build and Run

1. Ensure you have MPI installed on your system.
2. Compile the program using the provided Makefile:
   ```
   make
   ```
3. Run the program using `mpirun` command, specifying the number of processes and the number to be checked for primality:
   ```
   make run [number]
   ```
   Replace `[number]` with the integer you want to check for primality.
   
### Program Goal

The goal of this program is to efficiently determine whether a given number is prime by distributing the computation across multiple MPI processes. It utilizes parallel processing to accelerate the prime number checking process.

### License

This project is licensed under the MIT License. See the `LICENSE` file for details.

**MIT License**

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

