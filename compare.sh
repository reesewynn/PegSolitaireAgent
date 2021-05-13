#!/bin/bash

#SBATCH --partition=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --mem-per-cpu=5000
##SBATCH --exclusive
#SBATCH --job-name=Comparison
#SBATCH --output=Results/Comparison-%j.out

make

echo "COMPARE ALL METHODS"
export OMP_NUM_THREADS=32
export OMP_SCHEDULE="static"
echo "Number of threads = " $OMP_NUM_THREADS
echo "OMP_SCHEDULE = " $OMP_SCHEDULE

BOARD="   000 \
   101 \
 0000000 \
 1100011 \
 1111111 \
   111 \
   111 "

echo "$BOARD"
time ./play_peg parallel_dbb "${BOARD}"
