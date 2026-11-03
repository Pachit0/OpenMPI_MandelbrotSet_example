# OpenMPI_MandelbrotSet_example
## Description
This is an example solution for the MandelbrotSet in parallel using Master-Worker architecture. You can try different resolutions by changing the WIDTH and HEIGHT values in the program itself.

## Guide
1. Clone the repository
2. Compile requirements: gmp, mpi and make
3. Run on single PC: mpirun -np (number of processes) hello_mpi -v
4. Run on cluster: mpirun --hostfile /path_to/hostfile --map-by ppr:(number of processors per node):node -np (number of processes) hello_mpi -v "required hostfile for cluster"
5. Run with make -> make run_local or make run_cluster
