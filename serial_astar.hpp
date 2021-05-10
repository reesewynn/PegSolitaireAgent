#ifndef SERIAL_ASTAR_H
#define SERIAL_ASTAR_H

#include "peg_solitaire.hpp"
#include <stack>
#include <bitset>
#include <queue>
#include <limits>
#include <unordered_map>
#define INFINITY (std::numeric_limits<double>::infinity())

using std::stack, std::bitset, std::priority_queue, std::unordered_map;
using std::pair, std::vector;//, std::numeric_limits<double>::infinity;

#define compare_type float

typedef bitset<BOARD_SIZE> priority_queue_type;

class SerialAStarAgent {
private:
    compare_type getGScore(const priority_queue_type& board);
    compare_type getFScore(const priority_queue_type& board);

    stack<move_type> solution;
    bitset<BOARD_SIZE> goalState;
    PegSolitaire initBoard;
    

    unordered_map<priority_queue_type, compare_type> f, g;
    unordered_map<priority_queue_type, move_type> cameFrom;
    
    void buildPath(const priority_queue_type& endNode);

public:
    SerialAStarAgent();
    SerialAStarAgent(PegSolitaire &startingBoard);
    
    bool search();

    float heuristic(bitset<BOARD_SIZE> b);

    stack<move_type> getSolution();

};

#endif /* SERIAL_ASTAR_H */