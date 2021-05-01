#include "peg_solitaire.hpp"
#include "serial_search.hpp"
#include <iostream>
#include <stack>

using std::cout, std::cin, std::endl;
using std::stack;

// stack<move_type> path;

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
    stack<move_type> path = serial::backtrack(pegBoard);
    // TODO: if terminal from start, this is wrong.
    if (!path.empty()) {
        while (!path.empty()) {
             cout << path.top() << endl;
             path.pop();
        }
    }
    else {
        cout << "NO SOLUTION" << endl;
    }
}