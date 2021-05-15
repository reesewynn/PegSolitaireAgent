#include "../parallel_astar_critical.hpp"
#include <math.h>

compare_type ParallelAStarCriticalAgent::getGScore(const priority_queue_type& board) {
    return (g.contains(board))? g[board] : INFINITY;
}

compare_type ParallelAStarCriticalAgent::getFScore(const priority_queue_type& board) {
    return (f.contains(board))? f[board] : INFINITY;
}

void ParallelAStarCriticalAgent::initLocks() {
    omp_init_lock(&f_lock);
    omp_init_lock(&g_lock);
    omp_init_lock(&pq_lock);
    // omp_init_lock(&sol_lock);
}

ParallelAStarCriticalAgent::ParallelAStarCriticalAgent() {
    goalState.set(CENTER_IDX);
    bitset<BOARD_SIZE> b;
    for(int i = 0; i < BOARD_SIZE; i++) if (i != CENTER_IDX) b.set(i);

    initBoard.setState(b);
    initLocks();
}

ParallelAStarCriticalAgent::ParallelAStarCriticalAgent(PegSolitaire &startingBoard) {
    initBoard = startingBoard;
    goalState.set(CENTER_IDX);
    initLocks();
}

void ParallelAStarCriticalAgent::buildPath(const priority_queue_type& endNode) {
    PegSolitaire boardState = PegSolitaire(endNode);
    while (cameFrom.contains(boardState.getState())) {
        auto usedMove = cameFrom[boardState.getState()];
        solution.push(usedMove);
        boardState.undoMove(usedMove);
    }
}

bool ParallelAStarCriticalAgent::search() {
    auto cmp = [this](priority_queue_type const& a, priority_queue_type const& b) -> bool {
        return this->getFScore(a) >= this->getFScore(b);
    };
    priority_queue<priority_queue_type, vector<priority_queue_type>, decltype(cmp)> openSet(cmp);


    g[initBoard.getState()] = 0; // start with first state
    f[initBoard.getState()] = heuristic(initBoard.getState());

    openSet.push(initBoard.getState());
    unordered_map<priority_queue_type, bool> isInOpenSet;
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
        
        #pragma omp parallel for shared(openSet, isInOpenSet, g_lock, f_lock, pq_lock)
        for (int i = 0; i < size; i++) {
            auto move = moves[i];
            // apply move
            PegSolitaire newState = current; // doesn't recompute moves until ready
            newState.executeMove(move);
            auto state = newState.getState();


            compare_type tentativeGScore, gState;
            // tentative score
            #pragma omp critical (readG1)
            tentativeGScore = getGScore(current.getState()) + 1;
            
            // if better, record it!
            #pragma omp critical (readG2)
            gState = getGScore(state);

            if (tentativeGScore < gState) {
                #pragma omp critical (readG1)
                {
                    #pragma omp critical (readG2) 
                    {
                     #pragma omp critical (readG3)
                     {
                        // needs all locks!
                        g[state] = tentativeGScore;
                     }   
                    }
                }

                compare_type update = tentativeGScore + heuristic(state);

                #pragma omp critical (fScore)
                f[state] = update; 
                
                #pragma omp critical (openSet) 
                {
                    if (!isInOpenSet.contains(state)) {
                        // Performs a read on g 
                        #pragma omp critical (readG3)
                        {
                            openSet.push(state);

                        }
                        isInOpenSet[state] = true;
                    }
                }
                #pragma omp critical (cameFrom)
                cameFrom[state] = move;
            }
        }

    }

    return false;
}

compare_type ParallelAStarCriticalAgent::manhattan(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
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
compare_type ParallelAStarCriticalAgent::matched(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (eval[i] == goal[i]) count++;
    }
    return count;
}

compare_type ParallelAStarCriticalAgent::heuristic(bitset<BOARD_SIZE> b) {
    // return matched(goalState, b);
    return manhattan(goalState, b);
}

stack<move_type>& ParallelAStarCriticalAgent::getSolution() {
    return solution;
}