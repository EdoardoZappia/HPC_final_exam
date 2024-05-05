#!/bin/bash
#SBATCH --job-name=HPC
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=24
#SBATCH --time=02:00:00
#SBATCH --partition THIN
#SBATCH --exclusive
#SBATCH --exclude fat[001-002]
#SBATCH --exclude thin006

module load openMPI/4.1.5/gnu/12.2.1


mpirun -np 2 --cpu-list 0,1 osu_latency

