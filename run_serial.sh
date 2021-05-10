#!/bin/bash

#SBATCH --partition=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=01:00:00
#SBATCH --mem-per-cpu=5000
##SBATCH --exclusive
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

BOARD="  000 \
  101 \
0000000 \
1100011 \
1111111 \
  111 \
  111 "


echo "$BOARD"
time ./play_peg serial_backtrack "${BOARD}"
