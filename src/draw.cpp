#include "chess.h"

bool chess_t::is_repetition() {
    /*
    TODO: allow position to occur twice when first occurance is before search
    Optimizations:
    Start at last irreversible ply (always white) plus the side to move,
    ensuring it is the same side to move as right now.
    Skip by two as with other side to move repetition can't happen.
    */
    uint32_t count = 1;
    for (uint32_t i = board.last_irrev_ply + board.game_state_stack.last()->to_move;
         i < board.game_state_stack.size - 1; i += 2) {
        if (board.game_state_stack.last()->zobrist_key == board.game_state_stack[i].zobrist_key) {
            count++;
            if (count >= 3) {
                return true;
            }
        }
    }
    return false;
}

bool chess_t::is_insufficient_material() {
    uint64_t white_bishop = board.bitboards[WHITE][BISHOP];
    uint64_t black_bishop = board.bitboards[BLACK][BISHOP];

    uint64_t white_minor = board.bitboards[WHITE][KNIGHT] | white_bishop;
    uint64_t black_minor = board.bitboards[BLACK][KNIGHT] | black_bishop;

    uint64_t white = board.bitboards[WHITE][PAWN] | white_minor | board.bitboards[WHITE][ROOK] | board.bitboards[WHITE][QUEEN] | board.bitboards[WHITE][KING];
    uint64_t black = board.bitboards[BLACK][PAWN] | black_minor | board.bitboards[BLACK][ROOK] | board.bitboards[BLACK][QUEEN] | board.bitboards[BLACK][KING];

    // K vs K (assumes kings are on board)
    if (intrin::popcnt(white | black) == 2) {
        return true;
    }

    // K vs K(N or B)
    if (intrin::popcnt(black) == 1 &&
        intrin::popcnt(white) == 2 &&
        intrin::popcnt(white_minor) == 1) {
        return true;
    }


    if (intrin::popcnt(white) == 1 &&
        intrin::popcnt(black) == 2 &&
        intrin::popcnt(black_minor) == 1) {
        return true;
    }

    // KB vs KB (B on same color)
    // relies on short-circuiting as ctz with 0 is undefined in some implementations
    if (intrin::popcnt(white) == 2 &&
        intrin::popcnt(black) == 2 &&
        intrin::popcnt(white_bishop) == 1 &&
        intrin::popcnt(black_bishop) == 1 &&
        intrin::ctz(white_bishop) % 2 == intrin::ctz(black_bishop) % 2) {
        return true;
    }

    return false;
}