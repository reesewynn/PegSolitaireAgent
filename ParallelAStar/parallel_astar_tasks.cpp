#include "../parallel_astar.hpp"
#include <math.h>

compare_type ParallelAStarAgent::getGScore(const priority_queue_type& board) {
    return (g.contains(board))? g[board] : INFINITY;
}

compare_type ParallelAStarAgent::getFScore(const priority_queue_type& board) {
    return (f.contains(board))? f[board] : INFINITY;
}

void ParallelAStarAgent::initLocks() {
    omp_init_lock(&f_lock);
    omp_init_lock(&g_lock);
    omp_init_lock(&pq_lock);
    // omp_init_lock(&sol_lock);
}

ParallelAStarAgent::ParallelAStarAgent() {
    goalState.set(CENTER_IDX);
    bitset<BOARD_SIZE> b;
    for(int i = 0; i < BOARD_SIZE; i++) if (i != CENTER_IDX) b.set(i);

    initBoard.setState(b);
    initLocks();
}

ParallelAStarAgent::ParallelAStarAgent(PegSolitaire &startingBoard) {
    initBoard = startingBoard;
    goalState.set(CENTER_IDX);
    initLocks();
}

void ParallelAStarAgent::buildPath(const priority_queue_type& endNode) {
    PegSolitaire boardState = PegSolitaire(endNode);
    while (cameFrom.contains(boardState.getState())) {
        auto usedMove = cameFrom[boardState.getState()];
        solution.push(usedMove);
        boardState.undoMove(usedMove);
    }
}

bool ParallelAStarAgent::search() {
    // priority_queue<priority_queue_type, vector<priority_queue_type>, Compare> openSet(Compare(f));
    auto cmp = [this](priority_queue_type const& a, priority_queue_type const& b) -> bool {
        return this->getFScore(a) >= this->getFScore(b);
    };
    priority_queue<priority_queue_type, vector<priority_queue_type>, decltype(cmp)> openSet(cmp);

    typedef pair<priority_queue_type, compare_type> pq_entry;
    typedef pair<pq_entry, move_type> entry_move;
    vector<vector<entry_move>> threadAdditions;
    threadAdditions.reserve(omp_get_max_threads());
    for (int i = 0; i < omp_get_max_threads(); i++) {
        threadAdditions.push_back(vector<entry_move>());
        threadAdditions[i].reserve(60);
    }


    g[initBoard.getState()] = 0; // start with first state
    f[initBoard.getState()] = heuristic(initBoard.getState());

    openSet.push(initBoard.getState());
    unordered_map<priority_queue_type, bool> isInOpenSet;
    isInOpenSet[initBoard.getState()] = true;

    bool exit = false;
    priority_queue_type lastState;

    while (!openSet.empty() && !exit) {
        #pragma omp parallel shared(pq_lock, openSet, g, f, threadAdditions)
        {
            omp_set_lock(&pq_lock);
            if (!openSet.empty()) {
                PegSolitaire current = PegSolitaire(openSet.top()); // make new board, frustrating. I know.
                openSet.pop();
                omp_unset_lock(&pq_lock);
                if (current.isWon()) {
                    // buildPath(current.getState());
                    // return true;
                    lastState = current.getState();
                    exit = true;
                }
                isInOpenSet[current.getState()] = false;
                
                int rank = omp_get_thread_num();

                for (auto move : *current.getLegalMoves()) {
                    // apply move
                    PegSolitaire newState = current; // doesn't recompute moves until ready
                    newState.executeMove(move);
                    auto state = newState.getState();

                    // tentative score
                    compare_type tentativeGScore = getGScore(current.getState()) + 1;
                    
                    // if better, record it!
                    if (tentativeGScore < getGScore(state)) {
                        // g[state] = tentativeGScore;
                        // f[state] = tentativeGScore + heuristic(state);
                        // if (!isInOpenSet.contains(state)) {
                        //     openSet.push(state);
                        //     isInOpenSet[state] = true;
                        // }
                        // cameFrom[state] = move;
                        threadAdditions[rank].push_back(entry_move(pq_entry(state, tentativeGScore), move));
                    }
                }
            }
            else {
                omp_unset_lock(&pq_lock);
            }
        }
        if (exit) {
            buildPath(lastState);
            return true;
        }

        // now serial
        for (auto vec : threadAdditions) {
            if (vec.empty()) continue;
            for (auto addition : vec) {
                move_type move = addition.second;
                priority_queue_type state = addition.first.first;
                compare_type tentativeGScore = addition.first.second;

                if (tentativeGScore < getGScore(state)) {
                    g[state] = tentativeGScore;
                    f[state] = tentativeGScore + heuristic(state);
                    if (!isInOpenSet.contains(state)) {
                        openSet.push(state);
                        isInOpenSet[state] = true;
                    }
                    cameFrom[state] = move;
                }   
            }

            vec.clear();
        }

    }

    return false;
}

compare_type ParallelAStarAgent::manhattan(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
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
compare_type ParallelAStarAgent::matched(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (eval[i] == goal[i]) count++;
    }
    return count;
}

compare_type ParallelAStarAgent::heuristic(bitset<BOARD_SIZE> b) {
    // return matched(goalState, b);
    return manhattan(goalState, b);
}

stack<move_type> ParallelAStarAgent::getSolution() {
    return solution;
}