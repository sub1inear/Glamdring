#include "chess.h"
#include "data.h"
#include <iostream>

int main(int argc, char **argv) {
    chess_t chess;
    chess.board.load_fen("r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/2N2N2/PPPP1PPP/R1BQK2R w KQkq - 0 1");
    chess.board.print();
    chess_t::move_array_t array = chess.gen_moves();
    for (uint32_t i = 0; i < array.size; i++) {
        array[i].print();
        std::cout << '\n';
    }
}