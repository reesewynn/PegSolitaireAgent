#include "../parallel_astar_fan.hpp"

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

bool ParallelAStarAgent::singleSearch(PegSolitaire& curr) {
    // priority_queue<priority_queue_type, vector<priority_queue_type>, Compare> openSet(Compare(f));
    auto cmp = [this](priority_queue_type const& a, priority_queue_type const& b) -> bool {
        return this->getFScore(a) >= this->getFScore(b);
    };
    priority_queue<priority_queue_type, vector<priority_queue_type>, decltype(cmp)> openSet(cmp);


    g[curr.getState()] = 0; // start with first state
    f[curr.getState()] = heuristic(curr.getState());

    openSet.push(curr.getState());
    unordered_map<priority_queue_type, bool> isInOpenSet;
    isInOpenSet[curr.getState()] = true;

    while (!openSet.empty() && !(*flag)) {
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

bool ParallelAStarAgent::search() {

    vector<move_type> moves = *(initBoard.getLegalMoves());
    bool flag = false;

    #pragma omp parallel for shared(moves, flag)
    for(int i = 0; i < moves.size(); i++) {
        if (flag) continue;

        PegSolitaire copyBoard(initBoard.getState());
        copyBoard.executeMove(moves[i]);

        // generate serial a* agent, search
        SerialAStarAgent secretAgent(copyBoard);
        
        // if found, mark it
        if (secretAgent.search()) {
            #pragma omp critical 
            {
                if (!flag) {
                    flag = true;
                    std::cout << "FOUND\n";
                    solution = secretAgent.getSolution();
                }
            }
        }
    }
    
    return flag;
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