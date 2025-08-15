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
struct zobrist_data_t {
    uint64_t piece[2][6][64];
    uint64_t castling[2][2];
    uint64_t en_passant[8];
    uint64_t to_move;
};
struct zobrist_test_t {
    const char *fen;
    uint8_t moves_size;
    chess_t::move_t moves[7];
    uint64_t zobrist_key;
};
extern const char startpos_fen[];
extern const uint64_t king_castling_clear[][2];
extern const uint64_t king_castling_safe[][2];
extern const chess_t::square_t king_castling_start_squares[];
extern const chess_t::square_t king_castling_end_squares[][2];
extern const chess_t::square_t king_castling_end_squares_polyglot[][2];
extern const chess_t::square_t rook_castling_start_squares[][2];
extern const chess_t::square_t rook_castling_end_squares[][2];
extern const uint64_t pawn_attack_data[][64];
extern const perft_result_t perft_results[6];
extern const char piece_to_char[][7];
extern const int16_t piece_square_values[][64];
extern const uint8_t mvv_lva[][6];
extern const zobrist_data_t zobrist_random_data;
extern const zobrist_test_t zobrist_test_data[9];
}