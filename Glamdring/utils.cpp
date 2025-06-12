#include "chess.h"

chess_t::piece_color_t::piece_color_t(char c) {
    switch (tolower(c)) {
    case 'p':
        piece = PAWN;
        break;
    case 'n':
        piece = KNIGHT;
        break;
    case 'b':
        piece = BISHOP;
        break;
    case 'r':
        piece = ROOK;
        break;
    case 'q':
        piece = QUEEN;
        break;
    case 'k':
        piece = KING;
        break;
    default:
        piece = CLEAR;
        break;
    }
    color = (color_t)isupper(c);
}

chess_t::piece_color_t::operator char() {
    switch (piece) {
    case PAWN:
        return color == WHITE ? 'P' : 'p';
    case KNIGHT:
        return color == WHITE ? 'N' : 'n';
    case BISHOP:
        return color == WHITE ? 'B' : 'b';
    case ROOK:
        return color == WHITE ? 'R' : 'r';
    case QUEEN:
        return color == WHITE ? 'Q' : 'q';
    case KING:
        return color == WHITE ? 'K' : 'k';
    default:
        return '.';
    }
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
 
void chess_t::print_bitboard(uint64_t bitboard) {
    for (int32_t i = 7; i >= 0; i--) {
        for (int32_t j = 7; j >= 0; j--) {
            std::cout << ((bitboard >> ((i * 8 + j))) & 0x1) << ' ';
        }
        std::cout << '\n';
    }
}

int chess_t::test() {
    return 0;
}