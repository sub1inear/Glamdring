#pragma once
#include <cstdint>

struct magic_t {
    uint64_t magic;
    uint64_t mask;
    uint32_t idx;
    uint32_t shift;
};
extern const magic_t bishop_magics[];
extern const magic_t rook_magics[];
extern const uint64_t magic_move_data[];
extern const uint64_t knight_move_data[];
extern const uint64_t king_move_data[];