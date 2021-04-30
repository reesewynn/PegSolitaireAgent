#include "peg_solitaire.hpp"
#include <iostream>

using std::cout, std::cin, std::endl;


void backtrack(PegSolitaire &ps) {
    if (ps.isWon()) { // successful case
        cout << "Won!" << endl;
        return;
    }
    else if (ps.isTerminal()) { // no options and failed
        return;
    }

    // backup the moves to prevent recomputation
    shared_ptr<vector<move_type>> movesCopy(ps.getLegalMoves());
    for (move_type move : *movesCopy) { // try all moves
        ps.executeMove(move);
        backtrack(ps); // recursive backtracking
        ps.undoMove(move);
        ps.setLegalMoves(movesCopy); // uses smart pointers to avoid copying
    }
}

int main(int argc, char** argv) {
    PegSolitaire pegBoard;
    if (argc > 1) {
        string boardVal = argv[1];
        auto inputBoard = bitset<BOARD_SIZE>();
        int i = 0;
        for (auto val : boardVal) {
            if (val == '0' || val == '1') {
                if (val == '1') 
                    inputBoard.set(i);
                i++;
            }
        }
        if (i != BOARD_SIZE) {
            cout << "Invalid Board Input!" << endl;
            return 1;
        }
        pegBoard = PegSolitaire(inputBoard);
    }
    
    backtrack(pegBoard);
}