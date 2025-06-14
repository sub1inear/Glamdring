#include "data.h"

namespace data {
const uint64_t king_castling_data[][2] = {
    { 0x6000000000000000ull, 0xe00000000000000ull, },
    { 0x60ull, 0xeull }
};
const chess_t::square_t king_castling_end_squares[][2] = {
    { chess_t::G1, chess_t::C1, },
    { chess_t::G8, chess_t::C8, },
};
}