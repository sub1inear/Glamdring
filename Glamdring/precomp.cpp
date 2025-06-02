#include "chess.h"

static uint64_t gen_rook_mask(chess_t::square_t square) {
    uint64_t mask = 0;
    for (chess_t::square_t i = square - 8; i > 8; i -= 8) mask |= (uint64_t)1 << i;
    for (chess_t::square_t i = square + 8; i < 56; i += 8) mask |= (uint64_t)1 << i;
    for (chess_t::square_t i = square - 1; i % 8 > 0; i--) mask |= (uint64_t)1 << i;
    for (chess_t::square_t i = square + 1; i % 8 < 7 ; i++) mask |= (uint64_t)1 << i;
    return mask;
}
static uint64_t gen_rook_attacks(chess_t::square_t square, uint64_t blockers) {
    
}

void chess_t::gen_magics() {
    
}
