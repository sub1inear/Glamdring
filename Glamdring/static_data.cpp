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
const perft_result_t perft_results[] = {
    {
        "Position 3",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        {
            14,
            191,
            2812,
            43238,
            674624,
            11030083,
            178633661,
        },
    },
    {
        "Startpos",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        {
            20,
            400,
            8902,
            197281,
            4865609,
            119060324,
            3195901860,
        },
    },
    {
        "Kiwipete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        {
            48,
            2039,
            97862,
            4085603,
            193690690,
            8031647685,
            374190009323,
        },
    },
};
const char piece_to_char[][7] = {
    {
        'P',
        'N',
        'B',
        'R',
        'Q',
        'K',
        '.',
    },
    {
        'p',
        'n',
        'b',
        'r',
        'q',
        'k',
        '.',
    },
};
}