# OpenMPI_MandelbrotSet_example
## Description
This is an example solution for the MandelbrotSet in parallel using Master-Worker architecture with openmp and mpi. You can try different resolutions by changing the WIDTH and HEIGHT values in the program itself. Also the Makefile has rules for example run on 8 cores and 4 threads on each core, check the file if you want to thinker with the values.

## Guide
1. Clone the repository
2. Compile requirements: gmp, mpi and make, openmp
5. Run with make -> make run_local or make run_cluster

## NOTICE
If the program crashes open the Makefile and edit the number of processors from 8 to your specification.
