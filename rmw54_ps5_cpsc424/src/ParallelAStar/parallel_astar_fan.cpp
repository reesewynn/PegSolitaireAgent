#include "../parallel_astar_fan.hpp"
#include <math.h>

compare_type ParallelAStarFanAgent::getScore(unordered_map<priority_queue_type, compare_type> &m, const priority_queue_type& board) {
    return (m.contains(board))? m[board] : INFINITY;
}

void ParallelAStarFanAgent::initLocks() {
    omp_init_lock(&f_lock);
    omp_init_lock(&g_lock);
    omp_init_lock(&pq_lock);
    // omp_init_lock(&sol_lock);
}

ParallelAStarFanAgent::ParallelAStarFanAgent() {
    goalState.set(CENTER_IDX);
    bitset<BOARD_SIZE> b;
    for(int i = 0; i < BOARD_SIZE; i++) if (i != CENTER_IDX) b.set(i);

    initBoard.setState(b);
    initLocks();
}

ParallelAStarFanAgent::ParallelAStarFanAgent(PegSolitaire &startingBoard) {
    initBoard = startingBoard;
    goalState.set(CENTER_IDX);
    initLocks();
}

void ParallelAStarFanAgent::buildPath(const priority_queue_type& endNode, unordered_map<priority_queue_type, move_type>& cameFrom) {
    PegSolitaire boardState = PegSolitaire(endNode);
    while (cameFrom.contains(boardState.getState())) {
        auto usedMove = cameFrom[boardState.getState()];
        solution.push(usedMove);
        boardState.undoMove(usedMove);
    }
}

bool ParallelAStarFanAgent::singleSearch(PegSolitaire& curr) {

    unordered_map<priority_queue_type, compare_type> f, g;
    unordered_map<priority_queue_type, move_type> cameFrom;

    bool ret = false;

    g[curr.getState()] = 0; // start with first state
    f[curr.getState()] = heuristic(curr.getState());
    
    auto cmp = [this, &f](priority_queue_type const& a, priority_queue_type const& b) -> bool {
        return this->getScore(f, a) >= this->getScore(f, b);
    };
    priority_queue<priority_queue_type, vector<priority_queue_type>, decltype(cmp)> openSet(cmp);

    openSet.push(curr.getState());
    unordered_map<priority_queue_type, bool> isInOpenSet;
    isInOpenSet[curr.getState()] = true;

    while (!openSet.empty() && !(flag)) {
        // break;
        PegSolitaire current = PegSolitaire(openSet.top()); // make new board, frustrating. I know.
        openSet.pop();
        if (current.isWon()) {
            if (!flag) {
                #pragma atomic update
                flag = true;

                buildPath(current.getState(), cameFrom);
                ret = true;
            }
        }
        isInOpenSet[current.getState()] = false;
        if (flag) continue;
        for (auto move : *current.getLegalMoves()) {
            // apply move
            PegSolitaire newState = current; // doesn't recompute moves until ready
            newState.executeMove(move);
            auto state = newState.getState();

            // tentative score
            compare_type tentativeGScore = getScore(g, current.getState()) + 1;
            
            // if better, record it!
            if (tentativeGScore < getScore(g, state)) {
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

    return ret;
}

bool ParallelAStarFanAgent::search() {

    vector<move_type> moves = *(initBoard.getLegalMoves());
    int num_iters = moves.size();
    int useThreads = std::min(omp_get_max_threads(), num_iters);

    #pragma omp parallel for shared(moves, flag) num_threads(useThreads) 
    for(int i = 0; i < num_iters; i++) {
        if (flag) continue;

        PegSolitaire copyBoard(initBoard.getState());
        copyBoard.executeMove(moves[i]);

        singleSearch(copyBoard);
    }
    
    return flag;
}

compare_type ParallelAStarFanAgent::manhattan(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
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
compare_type ParallelAStarFanAgent::matched(const bitset<BOARD_SIZE>& goal, const bitset<BOARD_SIZE>& eval) {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (eval[i] == goal[i]) count++;
    }
    return count;
}

compare_type ParallelAStarFanAgent::heuristic(bitset<BOARD_SIZE> b) {
    return manhattan(goalState, b);
}

stack<move_type>& ParallelAStarFanAgent::getSolution() {
    return solution;
}