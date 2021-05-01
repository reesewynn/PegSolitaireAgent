#ifndef SERIAL_SEARCH_H
#define SERIAL_SEARCH_H

#include "peg_solitaire.hpp"
#include <stack>

using std::stack;

namespace serial {
    stack<move_type> backtrack(PegSolitaire &ps); 
}

#endif /* SERIAL_SEARCH_H */