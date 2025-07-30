#include "chess.h"
#include "data.h"

char chess_t::piece_to_char(piece_t piece) {
    return data::piece_to_char[BLACK][piece];
}

chess_t::piece_t chess_t::char_to_piece(char c) {
    for (uint32_t piece_idx = 0; piece_idx < 6; piece_idx++) {
        if (c == data::piece_to_char[BLACK][piece_idx]) {
            return (chess_t::piece_t)piece_idx;
        }
    }
    return CLEAR;
}

chess_t::piece_color_t::piece_color_t(char c) {
    for (uint32_t color_idx = 0; color_idx < 2; color_idx++) {
        for (uint32_t piece_idx = 0; piece_idx < 6; piece_idx++) {
            if (c == data::piece_to_char[color_idx][piece_idx]) {
                color = (color_t)color_idx;
                piece = (piece_t)piece_idx;
            }
        }
    }
}

chess_t::piece_color_t::operator char() {
    return data::piece_to_char[color][piece];
}

chess_t::square_t chess_t::file_rank_to_square(square_t file, square_t rank) {
    square_t square = ('8' - rank) * 8;
    square += file - 'a';
    return square;
}

void chess_t::square_to_file_rank(square_t square, char *out) {
    out[0] = 'a' + square % 8;
    out[1] = '8' - square / 8;
    out[2] = '\0';
}

void chess_t::print_square(square_t square, FILE *out) {
    putc('a' + square % 8, out);
    putc('8' - square / 8, out);
}
 
void chess_t::print_bitboard(uint64_t bitboard) {
    // TODO: use putchar instead of putc(*, stdout)
    for (uint32_t i = 0; i < 8; i++) {
        for (uint32_t j = 0; j < 8; j++) {
            putc('0' + ((bitboard >> (i * 8 + j)) & 0x1), stdout);
            putc(' ', stdout);
        }
        putc('\n', stdout);
    }
    putc('\n', stdout);
}