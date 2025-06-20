#include "chess.h"

uint32_t chess_t::perft(uint32_t depth, bool root) {
    uint32_t num_moves = 0;
    move_array_t moves = gen_moves();
    if (depth == 1) {
        return moves.size;
    }
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
    static constexpr uint32_t perft_results[] = {
        20,
        400,
        8902,
        197281,
        4865609,
        119060324,
        3195901860,
    };
    for (uint32_t i = 0; i < 7; i++) {
        std::cout << "Perft " << i + 1 << '\n';
        uint32_t perft_result = perft(i + 1);
        std::cout << "Nodes: " << perft_result << '\n';
        std::cout << "Expected Nodes: " << perft_results[i] << '\n';
        std::cout << (perft_result == perft_results[i] ? "Succeeded\n\n" : "Failed\n\n");
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::chrono::duration<float> time = end - start;
        std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(time).count() << " ms\n";
        std::cout << "NPS: " << std::fixed<< (uint32_t)(perft_result / time.count()) << std::scientific << "\n\n";
    }
    return 0;
}