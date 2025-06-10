#include "chess.h"
#include <iostream>

int main(int argc, char **argv) {
    chess_t chess;
    chess_t::print_bitboard(1243234234);
    std::cout << '\n';
    chess_t::print_bitboard(chess.gen_queen_moves(chess_t::E4, 1243234234));
}