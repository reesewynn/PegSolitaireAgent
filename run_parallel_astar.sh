#!/bin/bash

#SBATCH --partition=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --mem-per-cpu=5000
##SBATCH --exclusive
#SBATCH --job-name=ParallelAStar
#SBATCH --output=Results/ResultsParallelAStar-%j.out

make

echo "Parallel A Star Approach"
export OMP_NUM_THREADS=20
export OMP_SCHEDULE="static"
# echo "Number of threads = " $OMP_NUM_THREADS
# echo "OMP_SCHEDULE = " $OMP_SCHEDULE

# BOARD="   000 \
#    101 \
#  0000000 \
#  1100011 \
#  1111111 \
#    111 \
#    111 "
BOARD="011,101,1001111,1101111,1111111,111,111"

echo "$BOARD"

# for num_threads in 4 16 20
# do
#     export OMP_NUM_THREADS=$num_threads
#     ./play_peg parallel_astar_fan "${BOARD}" --num_threads $num_threads --csv
# done;

num_threads=20
./play_peg parallel_astar_lock "${BOARD}" --num_threads $num_threads --csv
# ./play_peg parallel_astar_critical "${BOARD}" --num_threads $num_threads --csv
# ./play_peg parallel_astar_task "${BOARD}" --num_threads $num_threads --csv
# ./play_peg parallel_astar_fan "${BOARD}" --num_threads $num_threads --csv
# ./play_peg parallel_astar_calculate "${BOARD}" --num_threads $num_threads --csv

echo "SERIAL A Star Approach"
./play_peg serial_astar "${BOARD}" --csv