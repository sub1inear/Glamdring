#include "chess.h"
#include "data.h"
#include <iostream>

int main(int argc, char **argv) {
    chess_t chess;
    chess.board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    chess.board.make_move({ chess_t::E2, chess_t::E4, chess_t::move_t::DOUBLE_PAWN_PUSH });
    chess.board.make_move({ chess_t::E7, chess_t::E5, chess_t::move_t::DOUBLE_PAWN_PUSH });
    chess.board.print();
}