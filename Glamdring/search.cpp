#include "chess.h"

int32_t chess_t::search(int32_t depth, bool root) {
    move_array_t moves = gen_moves();
    if (moves.size == 0) {
        return eval_min;
    }
    if (depth == 0) {
        return eval();
    }
    
    int32_t best_eval = eval_min;

    for (uint32_t i = 0; i < moves.size; i++) {
        move_t move = moves[i];
        
        board.make_move(move);
        int32_t move_eval = -search(depth - 1, false);
        board.undo_move(move);

        if (move_eval > best_eval) {
            best_eval = move_eval;
            if (root) {
                best_move = move;
            }
        }
    }
    return best_eval;
}