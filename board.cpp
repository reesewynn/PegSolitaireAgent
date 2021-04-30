#include "board.hpp"

template <typename Move, typename State>

bool Board<Move, State>::hasState(const State &check_state) {
    return gameState == check_state;
    // return false;
}
