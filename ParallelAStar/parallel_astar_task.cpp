#include "../parallel_astar_task.hpp"
#include <math.h>
// #include <boost>


compare_type ParallelAStarTaskAgent::getGScore(const priority_queue_type& board) {
    std::shared_lock< std::shared_mutex > r(_gaccess);
    return (g.contains(board))? g[board] : INFINITY;
}

compare_type ParallelAStarTaskAgent::getFScore(const priority_queue_type& board) {
    std::shared_lock< std::shared_mutex > r(_faccess);
    return (f.contains(board))? f[board] : INFINITY;
}

bool ParallelAStarTaskAgent::getIsInOpenSet(const priority_queue_type item) {
    std::shared_lock< std::shared_mutex > r(_iioaccess);
    return isInOpenSet.contains(item);
}

void ParallelAStarTaskAgent::writeGScore(const priority_queue_type& board, compare_type write) {
    std::unique_lock< std::shared_mutex > w(_gaccess);
    g[board] = write;

}
void ParallelAStarTaskAgent::writeFScore(const priority_queue_type& board, compare_type write) {
    std::unique_lock< std::shared_mutex > w(_faccess);
    g[board] = write;
}
void ParallelAStarTaskAgent::writeIsInOpenSet(const priority_queue_type& board, bool isIn) {
    std::unique_lock< std::shared_mutex > w(_iioaccess);
    isInOpenSet[board] = isIn;
}




void ParallelAStarTaskAgent::initLocks() {
    omp_init_lock(&pq_lock);
    num_pop = 5;
}

ParallelAStarTaskAgent::ParallelAStarTaskAgent() {
    goalState.set(CENTER_IDX);
    bitset<BOARD_SIZE> b;
    for(int i = 0; i < BOARD_SIZE; i++) if (i != CENTER_IDX) b.set(i);

    initBoard.setState(b);
    initLocks();
}

ParallelAStarTaskAgent::ParallelAStarTaskAgent(PegSolitaire &startingBoard) {
    initBoard = startingBoard;
    goalState.set(CENTER_IDX);
    initLocks();
}

void ParallelAStarTaskAgent::buildPath(const priority_queue_type& endNode) {
    PegSolitaire boardState = PegSolitaire(endNode);
    while (cameFrom.contains(boardState.getState())) {
        auto usedMove = cameFrom[boardState.getState()];
        solution.push(usedMove);
        boardState.undoMove(usedMove);
    }
}

bool ParallelAStarTaskAgent::search() {
    // priority_queue<priority_queue_type, vector<priority_queue_type>, Compare> openSet(Compare(f));
    auto cmp = [this](priority_queue_type const& a, priority_queue_type const& b) -> bool {
        return this->getFScore(a) >= this->getFScore(b);
    };
    priority_queue<priority_queue_type, vector<priority_queue_type>, decltype(cmp)> openSet(cmp);

    typedef pair<priority_queue_type, compare_type> pq_entry;
    typedef pair<move_type, compare_type> move_heuristic;
    typedef pair<pq_entry, move_heuristic> entry_move;
    vector<vector<entry_move>> threadAdditions;
    vector<vector<priority_queue_type>> threadStates;
    threadAdditions.reserve(omp_get_max_threads());
    threadStates.reserve(omp_get_max_threads());
    for (int i = 0; i < omp_get_max_threads(); i++) {
        threadAdditions.push_back(vector<entry_move>());
        threadStates.push_back(vector<priority_queue_type>());
        threadAdditions[i].reserve(60);
        threadStates[i].reserve(num_pop);
    }


    g[initBoard.getState()] = 0; // start with first state
    f[initBoard.getState()] = heuristic(initBoard.getState());

    openSet.push(initBoard.getState());
    // unordered_map<priority_queue_type, bool> isInOpenSet;
    isInOpenSet[initBoard.getState()] = true;

    bool exit = false;
    priority_queue_type lastState;

    while (!openSet.empty() && !exit) {
        #pragma omp parallel shared(pq_lock, openSet, g, f, threadAdditions, _gaccess, _faccess, _iioaccess)
        {   
            int rank = omp_get_thread_num();
            omp_set_lock(&pq_lock);
            if (!openSet.empty()) {
                int used = 0;
                for (int i = 0; i < num_pop; i++) {
                    if (!openSet.empty()) {
                        auto popped = openSet.top();
                        // if (popped.count() < 5)
                        //     std::cout << popped << std::endl;
                        threadStates[rank].push_back(std::move(popped));
                        openSet.pop();
                        used++;
                    }
                }
                omp_unset_lock(&pq_lock);
                for (int i = 0; i < used; i++) {
                    auto popped = threadStates[rank][i];
                    PegSolitaire current = PegSolitaire(popped); // make new board, frustrating. I know.
                    if (current.isWon()) {
                        lastState = current.getState();
                        exit = true;
                    }
                    // #pragma omp critical (isInOpen)
                    // isInOpenSet[current.getState()] = false;
                    writeIsInOpenSet(current.getState(), false);
                    
                    int rank = omp_get_thread_num();

                    for (auto move : *current.getLegalMoves()) {
                        // apply move
                        PegSolitaire newState = current; // doesn't recompute moves until ready
                        newState.executeMove(move);
                        auto state = newState.getState();

                        compare_type gCurr = 0, gNext = 0;

                        // tentative score
                        // #pragma omp critical (update)
                        gCurr = getGScore(current.getState());

                        compare_type tentativeGScore = gCurr + 1;
                        
                        // #pragma omp critical (update)
                        gNext = getGScore(state);

                        // if better, record it!
                        if (tentativeGScore < gNext) {
                            threadAdditions[rank].push_back(entry_move(pq_entry(state, tentativeGScore), move_heuristic(move, heuristic(state))));
                        }
                    }
                }

                for (auto entry : threadAdditions[rank]) {
                    priority_queue_type state = entry.first.first;
                    compare_type tentativeGScore = entry.first.second;
                    move_type move = entry.second.first;
                    compare_type h = entry.second.second;

                    compare_type gState = 0;
                    // #pragma omp critical (update)
                    gState = getGScore(state);


                    if (tentativeGScore < gState) {
                        // nest all readers to prevent concerrent issues
                        // #pragma omp critical (update) 
                        // {
                            // g[state] = tentativeGScore;
                            writeGScore(state, tentativeGScore);
                            // f[state] = tentativeGScore + h;
                            writeFScore(state, tentativeGScore + h);

                            // if (!isInOpenSet.contains(state)) {
                            if (!getIsInOpenSet(state)) {
                                #pragma omp critical (pushSet)
                                openSet.push(state);

                                // isInOpenSet[state] = true;
                                writeIsInOpenSet(state, true);
                            }
                        }

                        #pragma omp critical (cameFrom)
                        cameFrom[state] = move;
                    // }
                }

                threadAdditions[rank].clear();
                threadStates[rank].clear();
            }
            else {
                omp_unset_lock(&pq_lock);
            }
            #pragma omp barrier 
        }
       
        if (exit) {
            buildPath(lastState);
            return true;
        }
    }

    return false;
}

compare_type ParallelAStarTaskAgent::manhattan(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
    compare_type count = 0;
    // evaluate the manhattan distance for all UNMATCHED slots
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (eval[i] != goal[i]) {
            int row = GET_ROW(i);
            count += pow(2,std::max(abs(row - CENTER) , abs((i - row) - CENTER)));     
        }
    }

    return count;
}

compare_type ParallelAStarTaskAgent::matched(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (eval[i] != goal[i]) count++;
    }
    return count;
}

compare_type ParallelAStarTaskAgent::heuristic(bitset<BOARD_SIZE> b) {
    // return matched(goalState, b);
    return manhattan(goalState, b);
}

stack<move_type>& ParallelAStarTaskAgent::getSolution() {
    return solution;
}