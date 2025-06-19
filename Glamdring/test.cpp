#include "chess.h"

uint32_t chess_t::perft(uint32_t depth, bool root) {
    if (depth == 0) {
        return 1;
    }
    uint32_t num_moves = 0;
    move_array_t moves = gen_moves();
    for (uint32_t i = 0; i < moves.size; i++) {
        board.make_move(moves[i]);
        uint32_t new_moves = perft(depth - 1, false);
        if (root) {
            moves[i].print();
            std::cout << ": " << new_moves << '\n';
        }
        num_moves += new_moves;
        board.undo_move(moves[i]);
    }
    return num_moves;
}

uint32_t chess_t::test() {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    uint32_t perft_result = perft(6);
    std::cout << "Nodes: " << perft_result << '\n';
    // board.print();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<float> time = end - start;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(time).count() << " ms\n";
    std::cout << "NPS: " << std::fixed<< (uint32_t)(perft_result / time.count()) << std::scientific << '\n';
    return 0;
}