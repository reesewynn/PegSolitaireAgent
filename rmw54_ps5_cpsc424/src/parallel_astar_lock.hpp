#ifndef PARALLEL_ASTAR_LOCK_H
#define PARALLEL_ASTAR_LOCK_H

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

typedef bitset<BOARD_SIZE> entry_type;
typedef pair<entry_type, compare_type> pq_type;

class ParallelAStarLockAgent {
private:
    compare_type getGScore(const entry_type& board);
    compare_type getFScore(const entry_type& board);
    bool getIsInOpenSet(const entry_type item);

    void writeGScore(const entry_type& board, compare_type write);
    void writeFScore(const entry_type& board, compare_type write);
    void writeIsInOpenSet(const entry_type& board, bool isIn);


    void initLocks();

    stack<move_type> solution;
    bitset<BOARD_SIZE> goalState;
    PegSolitaire initBoard;
    

    unordered_map<entry_type, compare_type> f, g;
    unordered_map<entry_type, move_type> cameFrom;
    unordered_map<entry_type, bool> isInOpenSet;
    
    void buildPath(const entry_type& endNode);

    compare_type manhattan(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval);
    compare_type matched(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval);

    // FOR LOCK APPROACH
    omp_lock_t pq_lock;
    // https://stackoverflow.com/questions/989795/example-for-boost-shared-mutex-multiple-reads-one-write
    std::shared_mutex _gaccess, _faccess, _iioaccess;

public:
    ParallelAStarLockAgent();
    ParallelAStarLockAgent(PegSolitaire &startingBoard);
    
    bool search();

    compare_type heuristic(bitset<BOARD_SIZE> b);

    stack<move_type> &getSolution();

};

#endif /* PARALLEL_ASTAR_LOCK_H */