#include "parallel_dbb.hpp"

DepthBoundBranchingAgent::DepthBoundBranchingAgent(PegSolitaire &startingBoard) {
    initBoard = startingBoard;
}

void DepthBoundBranchingAgent::setInitBoard(PegSolitaire &newBoard) {
    initBoard = newBoard;
}

bool DepthBoundBranchingAgent::singleThreadBacktrack(PegSolitaire& threadBoard, int rank) {
    if (threadBoard.isWon()) { // successful case
        return true;
    }
    else if (threadBoard.isTerminal()) { // no options and failed
        return false;
    }

    // backup the moves to prevent recomputation
    shared_ptr<vector<move_type>> movesCopy(threadBoard.getLegalMoves());
    for (move_type move : *movesCopy) { // try all moves
        if (halt) return false;
        threadBoard.executeMove(move);
        if (singleThreadBacktrack(threadBoard, rank)) { // recursive backtracking
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

    // backup the moves to prevent recomputation
    shared_ptr<vector<move_type>> movesCopy = initBoard.getLegalMoves(); // was passed to the constructor
    if (depth == maxBranchDepth) {
        // fan out!
        // bool threadFoundSolutions[numThreads];
        // move_type lastMove[numThreads]; 
        // PegSolitaire threadBoards[numThreads]; // TODO: move to backTrack and make copy board instead

        for (int i = 0; i < numThreads; i++) {
            threadFoundSolutions[i] = false;

            // make copies of every board so they don't overwrite
            threadBoards[i] = initBoard;
            // threadBoards[i].setBoard(initBoard.getState());
            // threadBoards[i].setLegalMoves(movesCopy);
            // vector<move_type> copyMoves = *(initBoard.getLegalMoves());
            // shared_ptr<vector<move_type>> copyMovesPtr = &copyMoves;
            // threadBoards[i].setLegalMoves(make_shared<vector<move_type>>(copyMoves));
        }

        
        // bool flag = 0;
        // pragma for loop
        #pragma omp parallel for shared(threadFoundSolutions, threadBoards, halt, movesCopy) 
        for (int i = 0; i < movesCopy->size(); i++) {
            if (halt) continue;

            int rank = omp_get_thread_num();
            // execute move
            threadBoards[rank].executeMove((*movesCopy)[i]); 
            
            // let thread run
            threadFoundSolutions[rank] = threadFoundSolutions[rank] || singleThreadBacktrack(threadBoards[rank], rank);
            if (threadFoundSolutions[rank]) {
                // cout << "SHOULD STOP" << endl;
                halt = true; // don't care about interleaving.
            }
            // undo move
            threadBoards[rank].undoMove((*movesCopy)[i]);
            // initBoard.setLegalMoves(movesCopy); // uses smart pointers to avoid copying
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
                // cout << "FOUND ONE" << endl;
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
    threadBoards = new PegSolitaire[numThreads]; // TODO: move to backTrack and make copy board instead

    
    // while(threadSolutions.size() != numThreads) {
    for (int i = 0; i < numThreads; i++) {
        threadSolutions.push_back(stack<move_type>()); // TODO: compiles?
        threadBoards[i] = PegSolitaire();

    }

    
    bool ret = backtrackWithDepth(0);
    
    solFound = true;
    // delete[] threadFoundSolutions;
    // delete[] lastMoves;
    // delete[] threadBoards;

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
