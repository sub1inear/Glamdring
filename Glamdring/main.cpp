#include "chess.h"
#include "data.h"
#include <iostream>

int main(int argc, char **argv) {
    chess_t chess;
    chess.board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    chess.print_bitboard(chess.board.bitboards[chess_t::WHITE][chess_t::PAWN]);
}