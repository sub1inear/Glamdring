#pragma once
#include <cstdint>

struct pext_t {
    uint64_t mask;
    const uint64_t *ptr;
};
extern const pext_t rook_pext[];
extern const pext_t bishop_pext[];
extern const uint64_t pext_move_data[];
extern const uint64_t knight_move_data[];
extern const uint64_t king_move_data[];