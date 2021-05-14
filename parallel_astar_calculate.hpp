#ifndef PARALLEL_ASTAR_CALCULATE_H
#define PARALLEL_ASTAR_CALCULATE_H

#include "peg_solitaire.hpp"
#include <omp.h>
#include <stack>
#include <bitset>
#include <queue>
#include <limits>
#include <unordered_map>

#define INFINITY (std::numeric_limits<double>::infinity())

// using std::stack, std::bitset, std::priority_queue, std::unordered_map;
// using std::pair, std::vector;//, std::numeric_limits<double>::infinity;
using std::stack; using std::bitset; using std::priority_queue; using std::unordered_map;
using std::pair; using std::vector;//, std::numeric_limits<double>::infinity;


#define compare_type float

typedef bitset<BOARD_SIZE> priority_queue_type;

class ParallelAStarCalculateAgent {
private:
    compare_type getGScore(const priority_queue_type& board);
    compare_type getFScore(const priority_queue_type& board);

    void initLocks();

    stack<move_type> solution;
    bitset<BOARD_SIZE> goalState;
    PegSolitaire initBoard;
    

    unordered_map<priority_queue_type, compare_type> f, g;
    unordered_map<priority_queue_type, move_type> cameFrom;
    
    void buildPath(const priority_queue_type& endNode);

    compare_type manhattan(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval);
    compare_type matched(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval);

    // FOR LOCK APPROACH
    omp_lock_t f_lock, g_lock, pq_lock;
    // omp_lock_t sol_lock;

    int max_depth = 0;

    // FOR TASKS APPROACH
    // vector<pair<priority_queue_type&, compare_type>>& expandNode(PegSolitaire &node);

public:
    ParallelAStarCalculateAgent();
    ParallelAStarCalculateAgent(PegSolitaire &startingBoard);
    
    bool search();

    compare_type heuristic(bitset<BOARD_SIZE> b);

    stack<move_type> &getSolution();

};

#endif /* PARALLEL_ASTAR_CALCULATE_H */