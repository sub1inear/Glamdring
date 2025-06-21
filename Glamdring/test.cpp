#include "chess.h"
#include "data.h"

uint64_t chess_t::perft(uint32_t depth, bool root) {
    uint64_t num_moves = 0;
    move_array_t moves = gen_moves();
    if (depth == 1) {
        return moves.size;
    }
    for (uint32_t i = 0; i < moves.size; i++) {
        board.make_move(moves[i]);
        uint64_t new_moves = perft(depth - 1, false);
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
    /*board.load_fen("r3k2r/p2pqNb1/bnp1pnp1/3P4/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 2");
    board.print();
    std::cout << "Nodes: " << perft(2) << '\n';
    board.print();*/

    for (data::perft_result_t perft_pos : data::perft_results) {
        board.load_fen(perft_pos.fen);
        for (uint32_t i = 0; i < 7; i++) {
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

            std::cout << perft_pos.name << " Perft: " << i + 1 << '\n';
            uint64_t perft_result = perft(i + 1);
            
            std::cout << "Nodes: " << perft_result << '\n';
            std::cout << "Expected Nodes: " << perft_pos.results[i] << '\n';
            std::cout << (perft_result == perft_pos.results[i] ? "\x1b[32mSucceeded\x1b[0m\n\n" : "\x1b[31mFailed\x1b[0m\n\n");
            
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::chrono::duration<float> time = end - start;
            std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(time).count() << " ms\n";
            std::cout << "NPS: " << std::fixed << (uint64_t)(perft_result / time.count()) << std::scientific << "\n\n";
        }
    }
    return 0;
}