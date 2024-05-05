#!/bin/bash
#SBATCH --job-name=HPC
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=24
#SBATCH --time=00:30:00
#SBATCH --partition=THIN
#SBATCH --exclusive
#SBATCH --exclude thin006

module load openMPI/4.1.5/gnu/12.2.1
mpicc -o mandelbrot mandelbrot.c -lm -fopenmp

OMP_NUM_THREADS=1
mpirun -np 1 ./mandelbrot
