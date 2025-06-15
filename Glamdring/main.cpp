#include "chess.h"
#include "data.h"
#include <iostream>

int main(int argc, char **argv) {
    chess_t chess;

    chess.board.load_fen("4k3/8/4r3/8/4R3/8/8/4K3 b - - 0 1");
    chess.board.print();

    uint64_t blockers = chess.gen_blockers();
    uint64_t allies = chess.gen_allies();
    uint64_t enemies = blockers & ~allies;

    uint64_t danger = chess.gen_king_danger_squares(blockers);
    chess.print_bitboard(danger);
    chess.print_bitboard(chess.gen_pins(chess_t::E8, danger, allies, enemies));

}