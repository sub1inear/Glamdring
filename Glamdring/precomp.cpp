#include "chess.h"

// 
static uint64_t random64() {
    static uint64_t seed = 0xAB11CB56805F008D;
    seed = seed * 2862933555777941757 + 3037000493;
    return seed;
}
static uint64_t random64_low_bits() {
    return random64() & random64() & random64();
}

static uint64_t gen_rook_mask(chess_t::square_t square) {
    uint64_t mask = 0;
    for (chess_t::square_t i = square - 8; i > 8; i -= 8) mask |= (uint64_t)1 << i;
    for (chess_t::square_t i = square + 8; i < 56; i += 8) mask |= (uint64_t)1 << i;
    for (chess_t::square_t i = square - 1; i % 8 > 0; i--) mask |= (uint64_t)1 << i;
    for (chess_t::square_t i = square + 1; i % 8 < 7 ; i++) mask |= (uint64_t)1 << i;
    return mask;
}
static uint64_t gen_rook_moves(chess_t::square_t square, uint64_t blockers) {
    uint64_t moves = 0;
    for (chess_t::square_t i = square - 8; i > 8; i -= 8) {
        moves |= (uint64_t)1 << i;
        if (blockers & (uint64_t)1 << i) {
            break;
        }
    }
    for (chess_t::square_t i = square + 8; i < 56; i += 8) {
        moves |= (uint64_t)1 << i;
        if (blockers & (uint64_t)1 << i) {
            break;
        }
    }
    for (chess_t::square_t i = square - 1; i % 8 > 0; i--) {
        moves |= (uint64_t)1 << i;
        if (blockers & (uint64_t)1 << i) {
            break;
        }
    }
    for (chess_t::square_t i = square + 1; i % 8 < 7; i++) {
        moves |= (uint64_t)1 << i;
        if (blockers & (uint64_t)1 << i) {
            break;
        }
    }
    return moves;
}
static uint64_t gen_rook_magic(chess_t::square_t square, int shift /*< 12*/) {
    uint64_t rook_mask = gen_rook_mask(square);
    uint64_t rook_blockers_size = (uint64_t)1 << __popcnt64(rook_mask);
    
    uint64_t moves[4096];
    for (int i = 0; i < 1000000; i++) {
        bool succeeded = true;
        uint64_t magic = random64_low_bits();
        memset(moves, 0, sizeof(moves));
        for (uint64_t j = 0; j < rook_blockers_size; j++) {
            uint64_t blockers = _pdep_u64(i, rook_mask);
            uint64_t move = gen_rook_moves(square, blockers);
            uint64_t key = chess_t::magic_bitboard_key(blockers, magic, shift);
            if (moves[key] == 0) {
                moves[key] = move;
            } else if (moves[key] != move) {
                succeeded = false;
                break;
            }
        }
        if (succeeded) {
            return magic;
        }
    }
    return 0;
}
void chess_t::gen_magics() {
    std::cout << gen_rook_magic(E4, 10);
}
