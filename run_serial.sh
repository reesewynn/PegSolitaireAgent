#!/bin/bash

#SBATCH --partition=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=20:00:00
#SBATCH --mem-per-cpu=5000
#SBATCH --exclusive
#SBATCH --job-name=SerialBacktrack
#SBATCH --output=ResultsSerial-%j.out

make

echo "SERIAL BACKTRACKING APPROACH"

# BOARD="  111 
#   111 
# 1111111 
# 1110111 
# 1111111 
#   111 
#   111"

BOARD="  000 
  000 
0010100 
1110111 
1111111 
  111 
  111 "

time ./play_peg parallel_dbb "${BOARD}"
