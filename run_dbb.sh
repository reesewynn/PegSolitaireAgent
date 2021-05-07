#!/bin/bash

#SBATCH --partition=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=20:00:00
#SBATCH --mem-per-cpu=5000
#SBATCH --exclusive
#SBATCH --job-name=DepthBoundBranching
#SBATCH --output=ResultsDBB-%j.out

make

echo "Depth Bound Branching Approach"
export OMP_NUM_THREADS=32
export OMP_SCHEDULE="static"
echo "Number of threads = " $OMP_NUM_THREADS
echo "OMP_SCHEDULE = " $OMP_SCHEDULE

# BOARD="  111 
#   111 
# 1111111 
# 1110111 
# 1111111 
#   111 
#   111"

BOARD="  000 
  101 
0000000 
1100011 
1111111 
  111 
  111"

time ./play_peg parallel_dbb "${BOARD}"
