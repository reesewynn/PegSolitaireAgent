#include <iostream>
#include <stack>
#include <bitset>
#include <omp.h>

#include "peg_solitaire.hpp"


// using std::cout, std::stack, std::bitset, std::endl;
using std::cout; using std::stack; using std::bitset; using std::endl;

class DepthBoundBranchingAgent {

private:
    bitset<BOARD_SIZE> goalState;
    PegSolitaire initBoard;
    int numThreads = 32, maxBranchDepth = 5;
    bool *threadFoundSolutions;
    move_type *lastMoves;
    PegSolitaire *threadBoards;

    stack<move_type> solution;

    bool solFound = false, halt = false;

    vector<stack<move_type>> threadSolutions;


public:
    DepthBoundBranchingAgent(PegSolitaire &startingBoard);
    void setInitBoard(PegSolitaire &newBoard);

    bool singleThreadBacktrack(PegSolitaire& threadBoard, int rank);
    bool backtrackWithDepth(int depth);
    bool backtrack();

    const stack<move_type> &getSolution();
    bool getHaveFoundSolution();

};