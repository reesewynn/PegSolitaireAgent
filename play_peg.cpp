#include "peg_solitaire.hpp"
#include "serial_search.hpp"
#include "parallel_dbb.hpp"
#include "serial_astar.hpp"
#include "parallel_astar_lock.hpp"
#include "parallel_astar_critical.hpp"
#include "parallel_astar_calculate.hpp"
#include "parallel_astar_fan.hpp"
#include "parallel_astar_task.hpp"
#include <iostream>
#include <stack>
#include <cstring>
#include <stdio.h>

using std::cout, std::cin, std::endl;
using std::stack;

bool useCSV = false;
int numThreads = 1;

void print_solution(bool solFound, const stack<move_type> *solution, double elapsed_time, string name = "", int num_threads = numThreads) {
    if (!useCSV) {
        if (!solFound || solution == nullptr) {
            cout << "NO SOLUTION" << endl;
        }   
        else {
            // auto path = *solution;
            stack<move_type> path(*solution);
            while (!path.empty()) {
                cout << path.top() << endl;
                path.pop();
            }
        }
    }
    else {
        const char* foundMessage = (!solFound || solution == nullptr)? "false" : "true";
        int numRequired = (solution == nullptr)? -1 : solution->size(); 
        printf("%s,%d,%s,%.2e\n", name.c_str(), num_threads, foundMessage, elapsed_time);
    }
}

void serial_backtrack(PegSolitaire &pegBoard) {
    double start = omp_get_wtime();
    // auto path = serial::backtrack(pegBoard);
    SerialBacktrackAgent agent(pegBoard);
    bool solFound = agent.search();
    auto path = (solFound)? &(agent.getSolution()) : nullptr;
    print_solution(true, path, omp_get_wtime() - start, "Serial Backtracking", 1);
}

void parallel_dbb(PegSolitaire &pegBoard) {
    double start = omp_get_wtime();
    DepthBoundBranchingAgent agent(pegBoard);
    bool solFound = agent.backtrack();
    auto path = (solFound)? &(agent.getSolution()) : nullptr;
    
    print_solution(solFound, path, omp_get_wtime() - start, "Depth Bounded Branching", numThreads);
}

void serial_astar(PegSolitaire pegBoard) {
    try {
        double start = omp_get_wtime();
        SerialAStarAgent agent(pegBoard);
        bool solFound = agent.search();
        auto path = (solFound)? &(agent.getSolution()) : nullptr;
        
        print_solution(solFound, path, omp_get_wtime() - start, "Serial AStar", 1);
    } catch (...) { }
}

void parallel_astar_lock(PegSolitaire pegBoard) {
    try {
        double start = omp_get_wtime();
        ParallelAStarLockAgent agent(pegBoard);
        bool solFound = agent.search();
        auto path = (solFound)? &(agent.getSolution()) : nullptr;
        
        print_solution(solFound, path, omp_get_wtime() - start, "Parallel AStar With Locks", numThreads);
    } catch (...) { }
}

void parallel_astar_critical(PegSolitaire pegBoard) {
    double start = omp_get_wtime();
    ParallelAStarCriticalAgent agent(pegBoard);
    bool solFound = agent.search();
    auto path = (solFound)? &(agent.getSolution()) : nullptr;
    
    print_solution(solFound, path, omp_get_wtime() - start, "Parallel AStar With Critical Section", numThreads);
}

void parallel_astar_task(PegSolitaire pegBoard) {
    try {
        double start = omp_get_wtime();
        ParallelAStarTaskAgent agent(pegBoard);
        bool solFound = agent.search();
        auto path = (solFound)? &(agent.getSolution()) : nullptr;
        
        print_solution(solFound, path, omp_get_wtime() - start, "Parallel AStar With Task Section", numThreads);
    } catch (...) { }
}

void parallel_astar_calculate(PegSolitaire pegBoard) {
    double start = omp_get_wtime();
    ParallelAStarCalculateAgent agent(pegBoard);
    bool solFound = agent.search();
    auto path = (solFound)? &(agent.getSolution()) : nullptr;
    
    print_solution(solFound, path, omp_get_wtime() - start, "Parallel AStar With Parallel Heuristic", numThreads);
}

void parallel_astar_fan(PegSolitaire pegBoard) {
    double start = omp_get_wtime();
    ParallelAStarFanAgent agent(pegBoard);
    bool solFound = agent.search();
    auto path = (solFound)? &(agent.getSolution()) : nullptr;
    
    print_solution(solFound, path, omp_get_wtime() - start, "Parallel AStar Fan Method", numThreads);
}


int main(int argc, char** argv) {
    PegSolitaire pegBoard;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--csv")) {
            useCSV = true;
        }
        else if (i != argc - 1 && !strcmp(argv[i], "--num_threads")) {
            numThreads = atoi(argv[i + 1]);
        }
    }

    
    omp_set_num_threads(numThreads);

    if (argc > 2) {
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
    
    if (!strcmp(argv[1], "serial_backtrack")) {
        serial_backtrack(pegBoard);
    }
    else if (!strcmp(argv[1], "serial_astar")) {
        serial_astar(pegBoard);
    }
    else if (!strcmp(argv[1], "parallel_dbb")) {
        parallel_dbb(pegBoard);
    }
    else if (!strcmp(argv[1], "parallel_astar_lock")) {
        parallel_astar_lock(pegBoard);
    }
    else if (!strcmp(argv[1], "parallel_astar_critical")) {
        parallel_astar_critical(pegBoard);
    }
    else if (!strcmp(argv[1], "parallel_astar_task")) {
        parallel_astar_task(pegBoard);
    }
    else if (!strcmp(argv[1], "parallel_astar_calculate")) {
        parallel_astar_calculate(pegBoard);
    }
    else if (!strcmp(argv[1], "parallel_astar_fan")) {
        parallel_astar_fan(pegBoard);
    }
}