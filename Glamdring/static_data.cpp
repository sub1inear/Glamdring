#include "data.h"

namespace data {
const uint64_t king_castling_clear[][2] = {
    { 0x6000000000000000ull, 0xe00000000000000ull, },
    { 0x60ull, 0xeull }
};
const uint64_t king_castling_safe[][2] = {
    { 0x6000000000000000ull, 0xc00000000000000ull, },
    { 0x60ull, 0xcull }
};
const chess_t::square_t king_castling_end_squares[][2] = {
    { chess_t::G1, chess_t::C1, },
    { chess_t::G8, chess_t::C8, },
};
const chess_t::square_t rook_castling_start_squares[][2] = {
    { chess_t::H1, chess_t::A1 },
    { chess_t::H8, chess_t::A8 },
};
const chess_t::square_t rook_castling_end_squares[][2] = {
    { chess_t::F1, chess_t::D1 },
    { chess_t::F8, chess_t::D8 },
};
}