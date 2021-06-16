#!/bin/bash
module load mpi/openmpi3-x86_64

echo "Compiling Main.cpp..."
mpicxx -o main main.cpp
echo "Running Main.cpp..."
mpirun -np 4 main
