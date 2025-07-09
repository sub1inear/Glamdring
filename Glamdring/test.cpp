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

void chess_t::test_movegen() {
    uint32_t failures = 0;

    for (data::perft_result_t perft_pos : data::perft_results) {
        board.load_fen(perft_pos.fen);
        for (uint32_t i = 0; i < 7; i++) {
            std::cout << perft_pos.name << " Perft: " << i + 1 << '\n';
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            uint64_t perft_result = perft(i + 1);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            std::cout << "Nodes: " << perft_result << '\n';
            std::cout << "Expected Nodes: " << perft_pos.results[i] << '\n';
            bool failed = perft_result != perft_pos.results[i];
            failures += failed;
            std::cout << (failed ? "\x1b[31m" "Failed" "\x1b[0m" "\n\n" : "\x1b[32m" "Succeeded" "\x1b[0m" "\n\n");
            
            std::chrono::duration<float> time = end - start;
            std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(time).count() << " ms\n";
            std::cout << "NPS: " << std::fixed << (uint64_t)(perft_result / time.count()) << std::scientific << "\n\n";
        }
    }
    
    if (failures) {
        std::cout << "\x1b[31m" << failures << " tests failed." "\x1b[0m" "\n";
    } else {
        std::cout << "\x1b[32m" "All tests succeeded!" "\x1b[0m" "\n";
    }
}

template <typename T>
static bool assert(T expected, T result, const char *name) {
    if (memcmp(&expected, &result, sizeof(expected))) {
        std::cout << "\x1b[31m" "'" << name << "' test failed." "\x1b[0m" "\n";
        return true;
    }
    return false;
}

void chess_t::test_transposition_table() {
    uint32_t failures = 0;

    constexpr uint64_t key = 1000;

    constexpr uint8_t depth = 1;

    enum {
        UPPERBOUND_ALPHA,
        ALPHA,
        EXACT,
        BETA,
        LOWERBOUND_BETA,
    };

    constexpr move_t move = { 1, 2, move_t::QUIET };
    constexpr transposition_table_t::transposition_result_t upperbound_result = { move, ALPHA };
    constexpr transposition_table_t::transposition_result_t exact_result = { move, EXACT };
    constexpr transposition_table_t::transposition_result_t lowerbound_result = { move, BETA };

    transposition_table.store(exact_result, key, ALPHA, BETA, depth);
    transposition_table_t::transposition_result_t result = transposition_table.lookup(key, ALPHA, ALPHA, depth);
    failures += assert(exact_result, result, "Store (Exact Success)");
    
    transposition_table.table[key % transposition_table.size].data_xor_key = 0;
    result = transposition_table.lookup(key, ALPHA, ALPHA, depth);
    failures += assert(result.move.from, (packed_square_t)-1, "Corruption");

    transposition_table.store(upperbound_result, key, ALPHA, BETA, depth);
    result = transposition_table.lookup(key, UPPERBOUND_ALPHA, BETA, depth);
    failures += assert(result.move.from, (packed_square_t)-1, "Upperbound Failure");

    transposition_table.store(lowerbound_result, key, ALPHA, BETA, depth);
    result = transposition_table.lookup(key, ALPHA, LOWERBOUND_BETA, depth);
    failures += assert(result.move.from, (packed_square_t)-1, "Lowerbound Failure");


    if (failures) {
        std::cout << "\x1b[31m" << failures << " tests failed." "\x1b[0m" "\n";
    } else {
        std::cout << "\x1b[32m" "All tests succeeded!" "\x1b[0m" "\n";
    }
}