#include "chess.h"

chess_t::move_t chess_t::search() {
    move_array_t moves = gen_moves();
    return moves[0];
}