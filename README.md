# PegSolitaireAgent

## Background
This project was created by Reese Wynn in April 2021. 
The goal of this project is to analyze and compare serial and parallel implementations of search agents for the game Peg Solitaire.

### About Peg Solitaire
Peg Solitaire is a game over 300 years old played by royalty and citizens alike. It is a single player finite game consisting of 33 slots.
There are traditionally 32 slots filled with only a single empty slot in the center.
A peg may be "jumped" by an adjacent peg so long as the "jumping" peg moves into an empty slot opposite the "jumped" peg. 
This peg is now removed.
The game is over when a single peg remains in the center (a win) or there are no other moves (a loss). 
More information can be found at:
https://en.wikipedia.org/wiki/Peg_solitaire

## BUILD INSTRUCTIONS
This program requires C++20 or later. This utilizes the faster dictionary lookup and other niceties of the edition.
To build, simply run "make" from the parent director.
This will call the subdirectory makefiles and output an executable to the parent dir.

## MAIN PROGRAM USAGE
The main executable is "./play_peg"
Usage is as follows "./play_peg (mode) (initial_board) [--num_threads #] [--csv]

--num_threads specifies the max number of theads to use in execution
--csv turns off fancy printing to only output csv data

Optionally, you may add a destination board (if different from the traditional end goal) via
--end_goal BOARD
where BOARD is a string of 0's and 1's representing no peg or a peg at an index in the board
  NOTE: all non 0 or 1 characters are ignored. so use commas if they help!

  ex. --end_goal "111,111,1101010,1110111,0011101,010,010"


An example run is:
./play_peg parallel_dbb "111,111,1111111,1110111,1111111,111,111" --num_threads 20 --csv


## TASK RUN INSTRUCTIONS
Task 1: "sbatch run_serial_backtrack.sh"
Task 2: "sbatch run_dbb.sh"
Task 3: "sbatch run_serial_astar.sh"
Task 4.1: "sbatch run_parallel_astar_calcualte.sh"
Task 4.2: "sbatch run_parallel_astar_lock.sh"
Task 4.3: "sbatch run_parallel_astar_task.sh"
Task 4.4: "sbatch run_parallel_astar_fan.sh"

## INCLUDED CODE
peg_solitaire.hpp - Describes a board of peg solitaire. This represents the board state, legal moves, and the detection of a win or loss state.
play_peg.hpp - Can run a game of peg solitaire with the following search algorithms:
  1. Backtracking (SERIAL)
    Header: serial_search.hpp
    Implementation: serial_search.cpp

  2. Backtracking (PARALLEL)
    Header: parallel_dbb.hpp
    Implementation: parallel_dbb.hpp

  3. A* (SERIAL)
    Header: serial_astar.hpp
    Implementation: serial_astar.cpp

  4. A* (HEURISTIC LOOP PARALLELISM)
    Header: parallel_astar_calculate.hpp
    Implementation: parallel_astar_calculate.cpp

  5. A* (LOCK LOOP PARALLELISM)
    Header: parallel_astar_lock.hpp
    Implementation: parallel_astar_lock.cpp

  7. A* (TASK BASED)
    Header: parallel_astar_task.hpp
    Implementation: parallel_astar_task.cpp

  8. A* (TREE SPLITTING PARALLELISM)
    Header: parallel_astar_fan.hpp
    Implementation: parallel_astar_fan.cpp

  9. BOARD OBJECT
    Header: board.hpp
    Implementation: board.cpp

  10. PegSolitaire Object
    Header: peg_solitaire.hpp
    Implementation: peg_solitaire.cpp

  11. Main Program
    Implementation: play_peg.cpp


The list of tests is provided in Tests/test01.txt
