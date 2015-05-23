# MPI
This repository has some exercises that use MPI

How to install MPI in linux:

$ sudo apt-get install openmpi-bin openssh-client openssh-server libopenmpi-dev

How to compile with MPI:

$ mpic++ programName.cpp -o programName

How to run a program with MPI:

$ mpirun -np numberOfWorkers programName

Example:

$ mpirun -np 3 myprogram
