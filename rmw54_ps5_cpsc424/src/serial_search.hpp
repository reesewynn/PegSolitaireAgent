#ifndef SERIAL_SEARCH_H
#define SERIAL_SEARCH_H

#include "peg_solitaire.hpp"
#include <stack>
#include <unordered_map>

using std::cout; using std::stack; using std::bitset; using std::endl;
using std::unordered_map;

class SerialBacktrackAgent {

private:
    bitset<BOARD_SIZE> goalState;
    PegSolitaire initBoard;

    stack<move_type> solution;
 
    bool solFound = false;

public:
    SerialBacktrackAgent(PegSolitaire &startingBoard);
    void setInitBoard(PegSolitaire &newBoard);

    unordered_map<bitset<BOARD_SIZE>, bool> hasSeen;

    bool search();

    const stack<move_type> &getSolution();
    bool getHaveFoundSolution();

};

#endif /* PARALLEL_DBB_H */