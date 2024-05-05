#!/bin/bash
#SBATCH --job-name=HPC-OMP-WeakScaling
#SBATCH --nodes=1  # Utilizza un singolo nodo
#SBATCH --ntasks-per-node=1  # Un solo task per nodo
#SBATCH --cpus-per-task=24
#SBATCH --time=00:30:00
#SBATCH --partition=THIN
#SBATCH --exclusive
#SBATCH --exclude thin006

module load openMPI/4.1.5/gnu/12.2.1

export OMP_PROC_BIND=spread
export OMP_PLACES=cores
export OMP_DISPLAY_ENV=true
export OMP_VERBOSE=VERBOSE

# Definisci il nome del file CSV per salvare i risultati
OUTPUT_CSV="omp_weak_thin.csv"

# Inizializza il file CSV con l'intestazione
echo "OMP_NUM_THREADS,Problem_Size,Execution_Time" > $OUTPUT_CSV
# Dimensione di base del problema per 1 thread OpenMP
BASE_COLS=1000
BASE_ROWS=1000

for OMP_NUM_THREADS in {1..24}; do
    export OMP_NUM_THREADS
    # Aumenta la dimensione del problema proporzionalmente al numero di thread
    let cols=$BASE_COLS
    let rows=$((BASE_ROWS*OMP_NUM_THREADS))
    # Esegui il programma e salva il tempo di esecuzione
    execution_time=$(mpirun -np 1 --map-by socket --bind-to socket ./mandelbrot $cols $rows -2.0 -1.0 1.0 1.0 255 $OMP_NUM_THREADS)

    echo "$OMP_NUM_THREADS,$cols,$rows,$execution_time" >> $OUTPUT_CSV
done