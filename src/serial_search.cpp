#include "serial_search.hpp"

SerialBacktrackAgent::SerialBacktrackAgent(PegSolitaire &startingBoard) {
    initBoard = startingBoard;
}

void SerialBacktrackAgent::setInitBoard(PegSolitaire &newBoard) {
    initBoard = newBoard;
}

bool SerialBacktrackAgent::search() {
    if (initBoard.isWon()) { // successful case
        return true;
    }
    else if (initBoard.isTerminal()) { // no options and failed
        return false;
    }
    else if (hasSeen.contains(initBoard.getState())) {
        return false; // remove dupes  
    }

    hasSeen[initBoard.getState()] = true;

    // backup the moves to prevent recomputation
    shared_ptr<vector<move_type>> movesCopy(initBoard.getLegalMoves());
    for (move_type move : *movesCopy) { // try all moves
        initBoard.executeMove(move);
        if (search()) { // recursive backtracking
            solution.push(move);
            return true;
        } 
        initBoard.undoMove(move);
        initBoard.setLegalMoves(movesCopy); // uses smart pointers to avoid copying
    }
    return false;
}

const stack<move_type> &SerialBacktrackAgent::getSolution() {
    if (!solFound) {
        search();
    }
    return solution;
}

bool SerialBacktrackAgent::getHaveFoundSolution() {
    return solFound;
}
