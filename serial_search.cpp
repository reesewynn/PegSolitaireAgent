#include "serial_search.hpp"

namespace {
    stack<move_type> path;
    bool backtrackHelper(PegSolitaire &ps) {
        if (ps.isWon()) { // successful case
        return true;
        }
        else if (ps.isTerminal()) { // no options and failed
            return false;
        }

        // backup the moves to prevent recomputation
        shared_ptr<vector<move_type>> movesCopy(ps.getLegalMoves());
        for (move_type move : *movesCopy) { // try all moves
            ps.executeMove(move);
            if (backtrackHelper(ps)) { // recursive backtracking
                path.push(move);
                return true;
            } 
            ps.undoMove(move);
            ps.setLegalMoves(movesCopy); // uses smart pointers to avoid copying
        }
        return false;
    }
}

stack<move_type> serial::backtrack(PegSolitaire &ps) {
    while (!path.empty())
        path.pop();
    backtrackHelper(ps);
    return path;
}