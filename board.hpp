#ifndef BOARD_H
#define BOARD_H
#include <vector>
#include <memory>

using std::vector, std::shared_ptr;

template <typename Move, typename State>
class Board {
protected:
    State gameState;
public:
    virtual const shared_ptr<vector<Move>> getLegalMoves() = 0;

    virtual void executeMove(Move move) = 0;

    virtual bool isTerminal() = 0;

    bool hasState(const State &check_state);
};

#endif /* BOARD_H */