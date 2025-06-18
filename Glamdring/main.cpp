#include "chess.h"
#include "data.h"
#include <iostream>

int main(int argc, char **argv) {
    chess_t chess;

    chess.board.load_fen("4k3/8/4r1b1/7B/4R3/8/8/4K3 b - - 0 1");
    chess.board.print();

    uint64_t blockers = chess.gen_blockers();
    uint64_t allies = chess.gen_allies();
    uint64_t enemies = blockers & ~allies;
 
    uint64_t danger = chess.gen_king_danger_squares(blockers);
    uint64_t pin_lines[64];
    chess.gen_pins(pin_lines, chess_t::E8, danger, allies, enemies);
    
    chess.print_bitboard(pin_lines[chess_t::E6]);
    chess.print_bitboard(pin_lines[chess_t::G6]);

    //chess_t::move_array_t moves = chess.gen_moves();

    //for (uint32_t i = 0; i < moves.size; i++) {
    //    moves[i].print();
    //    std::cout << '\n';
    //}
}