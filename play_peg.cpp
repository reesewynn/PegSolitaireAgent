#include "peg_solitaire.hpp"
#include "serial_search.hpp"
#include "parallel_dbb.hpp"
#include "serial_astar.hpp"
#include "parallel_astar.hpp"
#include <iostream>
#include <stack>
#include <cstring>

using std::cout, std::cin, std::endl;
using std::stack;

// stack<move_type> path;

void serial_backtrack(PegSolitaire &pegBoard) {
    auto path = serial::backtrack(pegBoard);
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

void parallel_dbb(PegSolitaire &pegBoard) {
    DepthBoundBranchingAgent agent(pegBoard);
    if (agent.backtrack()) {
        auto path = agent.getSolution();
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
    else 
        cout << "NO SOLUTION" << endl;
}

void serial_astar(PegSolitaire pegBoard) {
    SerialAStarAgent agent(pegBoard);
    if (agent.search()) {
        auto path = agent.getSolution();
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
}

void parallel_astar(PegSolitaire pegBoard) {
    ParallelAStarAgent agent(pegBoard);
    if (agent.search()) {
        auto path = agent.getSolution();
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
    else {
        cout << "NO SOLUTION" << endl;
    }
}


int main(int argc, char** argv) {
    PegSolitaire pegBoard;
    if (argc > 1) {
        string boardVal = argv[2];
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
            cout << "Invalid Board Input! Size: " << i << endl;
            return 1;
        }
        pegBoard = PegSolitaire(inputBoard);
    }
    // stack<move_type> path; 
    // TODO: if terminal from start, this is wrong.
    if (!strcmp(argv[1], "serial_backtrack")) {
        serial_backtrack(pegBoard);
    }
    else if (!strcmp(argv[1], "serial_astar")) {
        serial_astar(pegBoard);
    }
    else if (!strcmp(argv[1], "parallel_dbb")) {
        parallel_dbb(pegBoard);
    }
    else if (!strcmp(argv[1], "parallel_astar")) {
        parallel_astar(pegBoard);
    }
}