# OpenMPI_MandelbrotSet_example
## Description
This is an example solution for the MandelbrotSet in parallel using Master-Worker architecture.

## Guide
1. Clone the repository
2. Compile requirements: gmp, mpi and make
3. Run on single PC: mpirun -np (number of processes) hello_mpi -v
4. Run for cluster: mpirun --hostfile /path_to/hostfile -np (number of processes) hello_mpi -v "required hostfile for cluster"
