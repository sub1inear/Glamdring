#include "chess.h"
#include "data.h"
#include <iostream>

int main(int argc, char **argv) {
    chess_t chess;
    chess.board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    chess.board.print();
    chess_t::move_array_t array = chess.gen_moves();
    for (uint32_t i = 0; i < array.size; i++) {
        array[i].print();
        std::cout << '\n';
    }
}