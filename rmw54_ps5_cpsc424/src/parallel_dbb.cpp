#include "parallel_dbb.hpp"

DepthBoundBranchingAgent::DepthBoundBranchingAgent(PegSolitaire &startingBoard, int setDepth) {
    initBoard = startingBoard;
    maxBranchDepth = 3;
}

void DepthBoundBranchingAgent::setInitBoard(PegSolitaire &newBoard) {
    initBoard = newBoard;
}

bool DepthBoundBranchingAgent::singleThreadBacktrack(PegSolitaire& threadBoard, int rank, seen_map_type& hasSeen) {
    if (threadBoard.isWon()) { // successful case
        return true;
    }
    else if (threadBoard.isTerminal()) { // no options and failed
        return false;
    }
    else if (hasSeen.contains(threadBoard.getState())) {
        return false; // remove dupes
    }

    hasSeen[threadBoard.getState()] = true;

    // backup the moves to prevent recomputation
    shared_ptr<vector<move_type>> movesCopy(threadBoard.getLegalMoves());
    for (move_type move : *movesCopy) { // try all moves
        if (halt) return false;
        threadBoard.executeMove(move);
        if (singleThreadBacktrack(threadBoard, rank, hasSeen)) { // recursive backtracking
            threadSolutions[rank].push(move);
            return true;
        } 
        threadBoard.undoMove(move);
        threadBoard.setLegalMoves(movesCopy); // uses smart pointers to avoid copying
    }
    return false;
}

bool DepthBoundBranchingAgent::backtrackWithDepth(int depth) {
    int rank = omp_get_thread_num();
    if (rank != 0) return false;
    if (initBoard.isWon()) { // successful case
        return true;
    }
    else if (initBoard.isTerminal()) { // no options and failed
        return false;
    }
    else if (globalSeen.contains(initBoard.getState())) {
        return false;
    }
    globalSeen[initBoard.getState()] = true;


    // backup the moves to prevent recomputation
    shared_ptr<vector<move_type>> movesCopy = initBoard.getLegalMoves(); // was passed to the constructor
    if (depth == maxBranchDepth) {
        // fan out!

        for (int i = 0; i < numThreads; i++) {
            threadFoundSolutions[i] = false;
            threadBoards[i] = initBoard; // make copies of every board so they don't overwrite
        }

        
        // pragma for loop
        #pragma omp parallel for shared(threadFoundSolutions, threadBoards, halt, movesCopy) 
        for (int i = 0; i < movesCopy->size(); i++) {
            if (halt) continue;

            int rank = omp_get_thread_num();
            // execute move
            threadBoards[rank].executeMove((*movesCopy)[i]); 
            seen_map_type threadMap;

            // let thread run
            threadFoundSolutions[rank] = \
                threadFoundSolutions[rank] || singleThreadBacktrack(threadBoards[rank], rank, threadMap);
            if (threadFoundSolutions[rank]) {
                halt = true; // don't care about interleaving.
            }
            // undo move
            threadBoards[rank].undoMove((*movesCopy)[i]);
            lastMoves[omp_get_thread_num()] = (*movesCopy)[i];
        }
        
        // single thread check if any where true, if so, which.
        for (int i = 0; i < numThreads; i++) {
            if (threadFoundSolutions[i]) {
                // merge stacks
                while(!threadSolutions[i].empty()) {
                    move_type move = threadSolutions[i].top();
                    solution.push(move);
                    threadSolutions[i].pop();
                }
                solution.push(move_type(lastMoves[i]));
                return true;
            }
        }
    }
    else {
        for (move_type move : *movesCopy) { // try all moves
            initBoard.executeMove(move);
            if (backtrackWithDepth(depth+1)) { // recursive backtracking
                solution.push(move);
                return true;
            } 
            initBoard.undoMove(move);
            initBoard.setLegalMoves(movesCopy); // uses smart pointers to avoid copying
        }
    }
    return false;
}

bool DepthBoundBranchingAgent::backtrack() {
    // setup all threads!
    numThreads = omp_get_max_threads();
    // cout << "Running on " << numThreads << " threads!" << endl;

    threadFoundSolutions = new bool[numThreads];
    lastMoves = new move_type[numThreads]; 
    threadBoards = new PegSolitaire[numThreads]; 

    
    // while(threadSolutions.size() != numThreads) {
    for (int i = 0; i < numThreads; i++) {
        threadSolutions.push_back(stack<move_type>()); 
        threadBoards[i] = PegSolitaire();

    }

    
    bool ret = backtrackWithDepth(0);
    
    solFound = true;
    return ret;
}   

const stack<move_type> &DepthBoundBranchingAgent::getSolution() {
    if (!solFound) {
        backtrack();
    }
    return solution;
}

bool DepthBoundBranchingAgent::getHaveFoundSolution() {
    return solFound;
}
