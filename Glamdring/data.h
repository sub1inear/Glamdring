#pragma once
#include <cstdint>
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

// static_data.cpp
extern const uint64_t king_castling_data[][2];
extern const chess_t::square_t king_castling_end_squares[][2];
}