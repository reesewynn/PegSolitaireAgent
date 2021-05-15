#ifndef PARALLEL_ASTAR_TASK_H
#define PARALLEL_ASTAR_TASK_H

#include "peg_solitaire.hpp"
#include <omp.h>
#include <stack>
#include <bitset>
#include <queue>
#include <limits>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#define INFINITY (std::numeric_limits<double>::infinity())

// using std::stack, std::bitset, std::priority_queue, std::unordered_map;
// using std::pair, std::vector;//, std::numeric_limits<double>::infinity;
using std::stack; using std::bitset; using std::priority_queue; using std::unordered_map;
using std::pair; using std::vector;//, std::numeric_limits<double>::infinity;


#define compare_type float

typedef bitset<BOARD_SIZE> priority_queue_type;

class ParallelAStarTaskAgent {
private:
    compare_type getGScore(const priority_queue_type& board);
    compare_type getFScore(const priority_queue_type& board);
    bool getIsInOpenSet(const priority_queue_type item);

    void writeGScore(const priority_queue_type& board, compare_type write);
    void writeFScore(const priority_queue_type& board, compare_type write);
    void writeIsInOpenSet(const priority_queue_type& board, bool isIn);

    void initLocks();

    stack<move_type> solution;
    bitset<BOARD_SIZE> goalState;
    PegSolitaire initBoard;
    

    unordered_map<priority_queue_type, compare_type> f, g;
    unordered_map<priority_queue_type, move_type> cameFrom;

    shared_ptr<bitset<BOARD_SIZE>> forCpy = nullptr;
    
    unordered_map<priority_queue_type, bool> isInOpenSet;
    
    void buildPath(const priority_queue_type& endNode);

    compare_type manhattan(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval);
    compare_type matched(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval);

    // FOR LOCK APPROACH
    omp_lock_t  pq_lock;
    // https://stackoverflow.com/questions/989795/example-for-boost-shared-mutex-multiple-reads-one-write
    //      adapted to use C++17 shared_mutex (much faster)
    std::shared_mutex _gaccess, _faccess, _iioaccess;

    int num_pop = 3;

public:
    ParallelAStarTaskAgent();
    ParallelAStarTaskAgent(PegSolitaire &startingBoard);
    
    bool search();

    compare_type heuristic(bitset<BOARD_SIZE> b);

    stack<move_type> &getSolution();

};

#endif /* PARALLEL_ASTAR_TASK_H */