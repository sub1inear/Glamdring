#pragma once
#include "chess.h"

namespace data {
// data.cpp
struct pext_t {
    uint64_t mask;
    const uint64_t *ptr;
};
extern const pext_t rook_pext[];
extern const pext_t bishop_pext[];
extern const uint64_t pext_move_data[];
extern const uint64_t knight_move_data[];
extern const uint64_t king_move_data[];
extern const uint64_t sliding_between_data[][64];

// static_data.cpp
struct perft_result_t {
    const char *name;
    const char *fen;
    uint64_t results[7];
};
extern const uint64_t king_castling_clear[][2];
extern const uint64_t king_castling_safe[][2];
extern const chess_t::square_t king_castling_end_squares[][2];
extern const chess_t::square_t rook_castling_start_squares[][2];
extern const chess_t::square_t rook_castling_end_squares[][2];
extern const uint64_t pawn_attack_data[][64];
extern const perft_result_t perft_results[3];
extern const char piece_to_char[][7];
}