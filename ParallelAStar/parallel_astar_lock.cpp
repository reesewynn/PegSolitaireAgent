#include "../parallel_astar_lock.hpp"
#include <math.h>

compare_type ParallelAStarLockAgent::getGScore(const priority_queue_type& board) {
    std::shared_lock< std::shared_mutex > r(_gaccess);
    return (g.contains(board))? g[board] : INFINITY;
}

compare_type ParallelAStarLockAgent::getFScore(const priority_queue_type& board) {
    std::shared_lock< std::shared_mutex > r(_faccess);
    return (f.contains(board))? f[board] : INFINITY;
}

bool ParallelAStarLockAgent::getIsInOpenSet(const priority_queue_type item) {
    std::shared_lock< std::shared_mutex > r(_iioaccess);
    return isInOpenSet.contains(item);
}

void ParallelAStarLockAgent::writeGScore(const priority_queue_type& board, compare_type write) {
    std::unique_lock< std::shared_mutex > w(_gaccess);
    g[board] = write;

}
void ParallelAStarLockAgent::writeFScore(const priority_queue_type& board, compare_type write) {
    std::unique_lock< std::shared_mutex > w(_faccess);
    g[board] = write;
}
void ParallelAStarLockAgent::writeIsInOpenSet(const priority_queue_type& board, bool isIn) {
    std::unique_lock< std::shared_mutex > w(_iioaccess);
    isInOpenSet[board] = isIn;
}

void ParallelAStarLockAgent::initLocks() {
    omp_init_lock(&pq_lock);
}

ParallelAStarLockAgent::ParallelAStarLockAgent() {
    goalState.set(CENTER_IDX);
    bitset<BOARD_SIZE> b;
    for(int i = 0; i < BOARD_SIZE; i++) if (i != CENTER_IDX) b.set(i);

    initBoard.setState(b);
    initLocks();
}

ParallelAStarLockAgent::ParallelAStarLockAgent(PegSolitaire &startingBoard) {
    initBoard = startingBoard;
    goalState.set(CENTER_IDX);
    initLocks();
}

void ParallelAStarLockAgent::buildPath(const priority_queue_type& endNode) {
    PegSolitaire boardState = PegSolitaire(endNode);
    while (cameFrom.contains(boardState.getState())) {
        auto usedMove = cameFrom[boardState.getState()];
        solution.push(usedMove);
        boardState.undoMove(usedMove);
    }
}

bool ParallelAStarLockAgent::search() {
    auto cmp = [this](priority_queue_type const& a, priority_queue_type const& b) -> bool {
        return this->getFScore(a) >= this->getFScore(b);
    };
    priority_queue<priority_queue_type, vector<priority_queue_type>, decltype(cmp)> openSet(cmp);


    g[initBoard.getState()] = 0; // start with first state
    f[initBoard.getState()] = heuristic(initBoard.getState());

    openSet.push(initBoard.getState());
    // unordered_map<priority_queue_type, bool> isInOpenSet;
    isInOpenSet[initBoard.getState()] = true;

    while (!openSet.empty()) {
        // break;
        PegSolitaire current = PegSolitaire(openSet.top()); // make new board, frustrating. I know.
        openSet.pop();
        if (current.isWon()) {
            buildPath(current.getState());
            return true;
        }
        isInOpenSet[current.getState()] = false;
        
        vector<move_type> moves = *current.getLegalMoves();
        int size = moves.size();
        
        #pragma omp parallel for shared(openSet, isInOpenSet, pq_lock)
        for (int i = 0; i < size; i++) {
            auto move = moves[i];
            // apply move
            PegSolitaire newState = current; // doesn't recompute moves until ready
            newState.executeMove(move);
            auto state = newState.getState();

            // tentative score
            compare_type tentativeGScore = getGScore(current.getState()) + 1;
            
            // if better, record it!
            if (tentativeGScore < getGScore(state)) {
                // omp_set_lock(&g_lock);
                // g[state] = tentativeGScore;
                writeGScore(state, tentativeGScore);
                // omp_unset_lock(&g_lock);


                // doesn't need to wait
                compare_type h = heuristic(state);
                
                // omp_set_lock(&f_lock);
                // f[state] = tentativeGScore + h;
                writeFScore(state, h + tentativeGScore);
                // omp_unset_lock(&f_lock);


                omp_set_lock(&pq_lock);
                // if (!isInOpenSet.contains(state)) {
                if (!getIsInOpenSet(state)) {
                    openSet.push(state);
                    isInOpenSet[state] = true;
                }
                
                omp_unset_lock(&pq_lock);

                #pragma omp critical (cameFrom)
                cameFrom[state] = move;
            }
        }

    }

    return false;
}

compare_type ParallelAStarLockAgent::manhattan(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
    compare_type count = 0;
    // evaluate the manhattan distance for all MATCHED slots
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (eval[i] != goal[i]) {
            int row = GET_ROW(i);
            count += pow(2,std::max(abs(row - CENTER) , abs((i - row) - CENTER)));     
        }
    }

    return count;
}
compare_type ParallelAStarLockAgent::matched(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (eval[i] == goal[i]) count++;
    }
    return count;
}

compare_type ParallelAStarLockAgent::heuristic(bitset<BOARD_SIZE> b) {
    // return matched(goalState, b);
    return manhattan(goalState, b);
}

stack<move_type>& ParallelAStarLockAgent::getSolution() {
    return solution;
}