#include "chess.h"
#include "data.h"

uint64_t chess_t::perft(uint32_t depth, bool root) {
    if (depth == 0) {
        return 1;
    }
    uint64_t num_moves = 0;
    move_array_t moves = gen_moves();
    
    for (move_t &move : moves) {
        board.make_move(move);
        uint64_t new_moves = perft(depth - 1, false);
        if (root) {
            move.print();
            printf(": %llu\n", new_moves);
        }
        num_moves += new_moves;
        board.undo_move(move);
    }
    return num_moves;
}

void chess_t::test_movegen() {
    uint32_t failures = 0;

    for (data::perft_result_t perft_pos : data::perft_results) {
        board.load_fen(perft_pos.fen); 
        for (uint32_t i = 0; i < 7; i++) {
            printf("%s Perft: %d\n", perft_pos.name, i + 1);
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            uint64_t perft_result = perft(i + 1);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            
            uint64_t expected_result = perft_pos.results[i];
            bool failed = perft_result != expected_result;
            failures += failed;
            
            std::chrono::duration<float> time = end - start;

            printf("Nodes: %llu\n"
                   "Expected Nodes: %llu\n"
                   "%s\n\n"
                   "Time: %lli ms\n"
                   "NPS: %llu\n\n",
                   perft_result,
                   expected_result,
                   failed ? "\x1b[31m" "Failed" "\x1b[0m": "\x1b[32m" "Succeeded" "\x1b[0m",
                   std::chrono::duration_cast<std::chrono::milliseconds>(time).count(),
                   (uint64_t)(perft_result / time.count())
            );
        }
    }
    
    if (failures) {
        printf("\x1b[31m"
               "%d tests failed."
               "\x1b[0m\n",
                failures
        );
    } else {
        puts("\x1b[32m"
              "All tests succeeded!"
              "\x1b[0m" // puts appends newline
        );
    }
}

template <typename T>
static bool assertf(T expected, T result, const char *fmt, ...) {
    if (expected != result) {
        va_list ap;
        va_start(ap, fmt);
        fputs("\x1b[31m" "'", stdout);
        vprintf(fmt, ap);
        puts("' test failed." "\x1b[0m\n");
        va_end(ap);
        return true;
    }
    return false;
}

void chess_t::test_transposition_table() {
    uint32_t failures = 0;

    constexpr uint64_t key = 1000;
    constexpr uint8_t depth = 1;
    constexpr uint8_t move_idx = 2;
    constexpr int32_t eval = 0;
    constexpr transposition_table_t::transposition_data_t expected_result = { eval, move_idx, depth, transposition_table_t::EXACT };

    transposition_table.store(eval, move_idx, key, -1, 1, depth);
    transposition_table_t::transposition_data_t result = transposition_table.lookup(key).data;
    failures += assertf(expected_result, result, "Store");

    for (data::zobrist_test_t zobrist_pos : data::zobrist_test_data) {
        board.load_fen(zobrist_pos.fen);
        failures += assertf(board.get_polyglot_key(), zobrist_pos.zobrist_key, zobrist_pos.fen);
        
        board.load_fen(data::startpos_fen);
        for (uint8_t i = 0; i < zobrist_pos.moves_size; i++) {
            board.make_move(zobrist_pos.moves[i]);
        }
        failures += assertf(board.get_polyglot_key(), zobrist_pos.zobrist_key, "%s Moves", zobrist_pos.fen);
    }


    if (failures) {
        printf("\x1b[31m"
               "%d tests failed."
               "\x1b[0m\n",
                failures
        );
    } else {
        puts("\x1b[32m"
              "All tests succeeded!"
              "\x1b[0m" // puts appends newline
        );
    }
}

void chess_t::test_draw() {
    uint32_t failures = 0;

    for (uint32_t i = 0; i < sizeof(data::repetition_test_data) / sizeof(data::repetition_test_data[0]); i++) {
        data::repetition_test_t repetition_pos = data::repetition_test_data[i];
        board.load_fen(repetition_pos.fen);
        for (uint8_t j = 0; j < repetition_pos.moves_size; j++) {
            board.make_move(repetition_pos.moves[j]);
        }
        failures += assertf(is_repetition(), repetition_pos.repetition, "%d Repetition", i);
    }

    for (data::insufficient_material_test_t insufficient_material_pos : data::insufficient_material_test_data) {
        board.load_fen(insufficient_material_pos.fen);
        failures += assertf(is_insufficient_material(), insufficient_material_pos.insufficient_material, insufficient_material_pos.fen);
    } 

    if (failures) {
        printf("\x1b[31m"
               "%d tests failed."
               "\x1b[0m\n",
                failures
        );
    } else {
        puts("\x1b[32m"
              "All tests succeeded!"
              "\x1b[0m" // puts appends newline
        );
    }
}