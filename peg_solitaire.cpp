#include "peg_solitaire.hpp"

PegSolitaire::PegSolitaire(bitset<BOARD_SIZE> inputBoard) {
    gameState = inputBoard;
    fillLegalMoves();
}

PegSolitaire::PegSolitaire() {
    int center = ROW_IDX(CENTER) + CENTER;
    // center += CENTER;
    // int temp = CENTER;
    gameState = bitset<BOARD_SIZE>(1);
    for(int i = 0; i < BOARD_SIZE; i++) {
        gameState[i] = (i == center)? 0 : 1;
    }
    fillLegalMoves();
}

const shared_ptr<vector<move_type>> PegSolitaire::getLegalMoves() {
    return legalMoves;
}

bool isValidLocation(const location& dest) {
    if(dest.first < 0 || dest.first >= MAX_ROW) {
        return false;
    }
    else if (dest.second < 0 || dest.second >= MAX_COL_WIDE) {
        return false;
    }
    else if ((dest.first < MIDDLE_LEFT || dest.first >= MIDDLE_RIGHT) && dest.second >= MAX_COL_SMALL) {
        return false;
    }
    else if (ROW_IDX(dest.first) + dest.second >= BOARD_SIZE) {
        return false;
    }

    return true;
}

location PegSolitaire::findDest(const move_type& move, int hop_count) {
    int dy = (move.second == EAST)? 1 : (move.second == WEST)? -1 : 0;
    int dx = (move.second == NORTH)? -1 : (move.second == SOUTH)? 1 : 0;

    const int move_x = (move.first).first, move_y = move.first.second;
    int offset = DY_OFFSET(move_x, (dx * hop_count)); // necessary for indexing
    return location(move_x + (dx * hop_count), move_y + (dy * hop_count) + offset); // *2 for hop
}

bool PegSolitaire::isLegalMove(const move_type& move) {
    const location dest = this->findDest(move, 2); // orig: no 2
    int dest_x = dest.first, dest_y = dest.second;

    int dy = (move.second == EAST)? 1 : (move.second == WEST)? -1 : 0;
    int dx = (move.second == NORTH)? -1 : (move.second == SOUTH)? 1 : 0;

    int init_x = (move.first).first, init_y = move.first.second;
    dy += DY_OFFSET(init_x, dx);

    int isHopped = ROW_IDX(init_x + dx) + (init_y + dy);
    int hopTo = ROW_IDX(dest_x) + dest_y;
    if (!isValidLocation(location(init_x, init_y)) || !isValidLocation(dest)) {
        return false; // invalid start or end
    }
    else if (isHopped >= BOARD_SIZE || isHopped < 0 || !this->gameState.test(isHopped)) {
        return false; // cannot be hopped
    }
    else if (hopTo >= BOARD_SIZE || hopTo < 0 || this->gameState.test(hopTo)) {
        return false;
    }

    return true; // valid
}

void PegSolitaire::executeMove(move_type move) {
    if (!this->isLegalMove(move)) {
        throw "Out of bounds";
    }
    
    // flip all three affected positions!
    this->gameState.flip(ROW_IDX(move.first.first) + move.first.second); // start
    location to_hop = findDest(move, 1);
    this->gameState.flip(ROW_IDX(to_hop.first) + to_hop.second);
    to_hop = findDest(move, 2);
    this->gameState.flip(ROW_IDX(to_hop.first) + to_hop.second);
    movesFound = false;
    //TODO: update moves around this move. 
}

void PegSolitaire::undoMove(move_type move) {
    // TODO: check legality
    // flip all three affected positions!
    this->gameState.flip(ROW_IDX(move.first.first) + move.first.second); // start
    location to_hop = findDest(move, 1);
    this->gameState.flip(ROW_IDX(to_hop.first) + to_hop.second);
    to_hop = findDest(move, 2);
    this->gameState.flip(ROW_IDX(to_hop.first) + to_hop.second);
    movesFound = false;
    //TODO: update moves around this move. 
}

void PegSolitaire::fillLegalMoves() {
    legalMoves = shared_ptr<vector<move_type>>(new vector<move_type>);
    int num_cols = MAX_COL_SMALL;
    for(int row = 0; row < MAX_ROW; row++) {
        num_cols = (row < MIDDLE_RIGHT && row >= MIDDLE_LEFT)? MAX_COL_WIDE : MAX_COL_SMALL;
        for(int col = 0; col < num_cols; col++) {
            if (gameState.test(ROW_IDX(row) + col)) {
                // try all four directions, if available, add. 
                location pos = location(row, col);
                for(int dir = NORTH; dir != WEST + 1; dir++) {
                    move_type move = move_type(pos, (direction) dir);
                    if (isLegalMove(move)) {
                        legalMoves->push_back(move);
                    }
                }
            }
        }
    }
    movesFound = true;
    // TODO: avoid ROW_IDX macro, by iterating with += 3, and if (middle) += 2 again
}

bool PegSolitaire::isTerminal() {
    if (!movesFound) {
        // find moves
        fillLegalMoves();
    }
    return legalMoves->size() == 0;
}

bool PegSolitaire::isWon() {
    if (gameState[CENTER_IDX] && gameState.count() == 1) {
        return true;
    }
    return false;
}

void PegSolitaire::setLegalMoves(shared_ptr<vector<move_type>> new_moves) {
    legalMoves = new_moves;
    movesFound = true;    
}


ostream& operator<<(ostream &os, const move_type& move) {
    os << "x: " << move.first.first << " y: " << move.first.second \
        << " dir: " << move.second;
    return os;
}

ostream& operator<<(ostream &os, const direction& dir) {
    switch (dir) {
        case NORTH:
            os << "NORTH";
            break;
        case EAST:
            os << "EAST";
            break;
        case SOUTH:
            os << "SOUTH";
            break;
        case WEST:
            os << "WEST";
            break;
        default:
            os << "INVALID DIR";
            break;
    }
    return os;
}

void PegSolitaire::setState(bitset<BOARD_SIZE> newState) {
    this->gameState = newState;
}
bitset<BOARD_SIZE> PegSolitaire::getState() {
    return gameState;
}