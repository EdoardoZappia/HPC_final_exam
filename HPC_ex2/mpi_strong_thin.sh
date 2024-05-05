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

export OMP_NUM_THREADS=1

# Nome del file CSV per salvare i tempi di esecuzione
output_file="mpi_strong_thin.csv"

# Creiamo l'intestazione del file CSV
echo "Total Tasks,Execution Time (s)" > $output_file

# Loop attraverso un numero variabile di task MPI totali
for total_tasks in {1..48}; do
    echo "Running with $total_tasks MPI tasks."

    # Esegui mpirun e cattura il tempo di esecuzione
    execution_time=$(mpirun -np $total_tasks ./mandelbrot 2400 1600 -2.0 -1.0 1.0 1.0 255 $OMP_NUM_THREADS)

    echo "$total_tasks,$execution_time" >> $output_file

done