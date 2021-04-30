#ifndef PEG_SOLITAIRE_H
#define PEG_SOLITAIRE_H

#include <bitset>
#include <iostream>
#include <memory>
#include "board.hpp"

using std::bitset, std::string, std::pair, std::ostream;
using std::shared_ptr;

#define BOARD_SIZE 33
#define MAX_ROW 7
#define MAX_COL_SMALL 3
#define MAX_COL_WIDE 7
#define MIDDLE_LEFT 2
#define MIDDLE_RIGHT 5
#define CENTER 3

#define TOP_ROWS_COUNT (MAX_COL_SMALL * (MAX_COL_SMALL - 1))
#define TOP_AND_MIDDLE_COUNT (TOP_ROWS_COUNT + (MAX_COL_SMALL * MAX_COL_WIDE))

// TODO: Validate
#define ROW_IDX(x) (((x) < MIDDLE_LEFT) ? \
    (x) * MAX_COL_SMALL : \
    ((x) < MIDDLE_RIGHT)? (TOP_ROWS_COUNT + ((x) - (MAX_COL_SMALL-1)) * MAX_ROW) : \
    TOP_AND_MIDDLE_COUNT + ((x) - MIDDLE_RIGHT) * MAX_COL_SMALL)


#define DY_OFFSET(x, dx) \
    (((x == MIDDLE_LEFT - 1 && dx == 1) || (x == MIDDLE_RIGHT && dx == -1))? \
        MIDDLE_LEFT : \
        0)


#define CENTER_IDX (ROW_IDX(CENTER) + CENTER)

enum direction {
    NORTH = 0,
    EAST = 1, 
    SOUTH = 2,
    WEST = 3
};

typedef pair<int, int> location;
typedef pair<location, direction> move_type;

class PegSolitaire : public Board<move_type, bitset<BOARD_SIZE>> {
private:
    location findDest(const move_type& move, int hop_count = 2);
    bool isLegalMove(const move_type& move);
    shared_ptr<vector<move_type>> legalMoves;
    bool movesFound = false;
    void fillLegalMoves();
public:
    PegSolitaire();
    PegSolitaire(bitset<BOARD_SIZE> inputBoard);
    const shared_ptr<vector<move_type>> getLegalMoves() override;

    void executeMove(move_type move) override;
    void undoMove(move_type move);

    bool isTerminal() override;
    bool isWon();
    void setLegalMoves(shared_ptr<vector<move_type>> new_moves); 
};


ostream& operator<<(ostream &os, const move_type& move); 
ostream& operator<<(ostream &os, const direction& dir); 

#endif /* PEG_SOLITAIRE_H */