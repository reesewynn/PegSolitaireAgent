#include "../parallel_astar.hpp"

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
        
        for (auto move : *current.getLegalMoves()) {
            // apply move
            PegSolitaire newState = current; // doesn't recompute moves until ready
            newState.executeMove(move);
            auto state = newState.getState();

            // tentative score
            compare_type tentativeGScore = getGScore(current.getState()) + 1;
            
            // if better, record it!
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

    }

    return false;
}

compare_type ParallelAStarAgent::manhattan(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
    compare_type count = 0;
    // evaluate the manhattan distance for all MATCHED slots
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (eval[i] == goal[i]) {
            int row = GET_ROW(i);
            count += (row - CENTER + (i - row) - CENTER);     
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