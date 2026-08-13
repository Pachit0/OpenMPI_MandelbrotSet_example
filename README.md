# OpenMPI_MandelbrotSet_example
## Description
This is an example solution for the MandelbrotSet in parallel using Master-Worker architecture.

## Guide
1. Clone the repository
2. Compile: mpicc -o hello_mpi hello_mpi.c -lm -lgmp "required libs: gmp and mpi"
3. Run: mpirun -np 4 hello_mpi -v or mpirun --hostfile /path_to/hostfile -np 100 hello_mpi -v "required hostfile for cluster"
