#!/bin/bash

#SBATCH --partition=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --threads-per-core=1
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --mem-per-cpu=5G
#SBATCH --exclusive
#SBATCH --job-name=ParallelAStarTask
#SBATCH --output=Tests/Unsolvable/Results/ResultsParallelAStarTask-%j.out

cd src && make

echo "Parallel A Star Task Approach"

TEST_CASE1=110,000,0000000,0110110,0000000,000,011
TEST_CASE2=110,000,0000000,1100011,0000000,000,011
TEST_CASE3=000,000,0000000,0100000,0000000,000,000
TEST_CASE4=111,111,1111111,1110111,1111111,000,001
TEST_CASE5=111,111,0000000,0000110,1100111,000,000
TEST_CASE6=100,000,1111111,1110111,1111111,000,000


BOARDS=($TEST_CASE1 $TEST_CASE2 $TEST_CASE3 $TEST_CASE4 $TEST_CASE5 $TEST_CASE6)

for BOARD in "${BOARDS[@]}";
do
    for num_threads in 4 16 20 
    do
        export OMP_NUM_THREADS=$num_threads
        ./play_peg parallel_astar_task "${BOARD}" --num_threads $num_threads --csv
    done;
done;

