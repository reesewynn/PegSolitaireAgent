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
        
        vector<move_type> moves = *current.getLegalMoves();
        int size = moves.size();
        
        #pragma omp parallel for shared(openSet, isInOpenSet, g_lock, f_lock, pq_lock)
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
                omp_set_lock(&g_lock);
                g[state] = tentativeGScore;
                omp_unset_lock(&g_lock);
                omp_set_lock(&f_lock);
                f[state] = tentativeGScore + heuristic(state);
                omp_unset_lock(&f_lock);
                omp_set_lock(&pq_lock);
                if (!isInOpenSet.contains(state)) {
                    openSet.push(state);
                    isInOpenSet[state] = true;
                }
                cameFrom[state] = move;
                omp_unset_lock(&pq_lock);
            }
        }

    }

    return false;
}

float ParallelAStarAgent::heuristic(bitset<BOARD_SIZE> b) {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (b[i] == goalState[i]) count++;
    }
    return count;
}

stack<move_type> ParallelAStarAgent::getSolution() {
    return solution;
}