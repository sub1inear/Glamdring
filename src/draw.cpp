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