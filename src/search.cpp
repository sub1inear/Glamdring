#include "chess.h"
#include "data.h"

chess_t::move_t chess_t::order_moves(move_array_t &moves, uint8_t (&scores)[max_moves], uint32_t i) {
    // selection sort is O(n^2) but becomes O(n) with beta cutoff
    // inspired by https://rustic-chess.org/search/ordering/how.html
    for (uint32_t j = i + 1; j < moves.size; j++) {
        if (scores[j] > scores[i]) {
            std::swap(moves[j], moves[i]);
            std::swap(scores[j], scores[i]);
        }
    }
    return moves[i];
}

int32_t chess_t::negamax(uint32_t depth, uint64_t max_nodes, bool root, int32_t alpha, int32_t beta) {
    if (!searching) {
        return 0;
    }

    // lookup in transposition table and return if entry matches constraints
    uint64_t zobrist_key = board.game_state_stack.last()->zobrist_key;
    transposition_table_t::transposition_entry_t entry = transposition_table.lookup(zobrist_key);
    
    bool entry_valid = entry.data_xor_key == ((uint64_t)entry.data ^ zobrist_key);

    // TODO: add return move at root along with a move validity check
    if (entry_valid && entry.data.depth >= depth && !root) {
        switch (entry.data.type) {
        case transposition_table_t::EXACT:
            return entry.data.eval;
        case transposition_table_t::UPPERBOUND:
            if (entry.data.eval <= alpha) {
                return entry.data.eval;
            }
            break;
        case transposition_table_t::LOWERBOUND:
            if (entry.data.eval >= beta) {
                return entry.data.eval;
            }
            break;
        }
    }

    // TODO: search transposition table entry first without involving move ordering
    move_array_t moves = gen_moves();

    if (is_repetition() || is_insufficient_material()) {
        return 0;
    }

    // TODO: check fifty-move rule

    if (moves.size == 0) {
        // TODO: return 0 if not in check
        return eval_min;
    }

    if (depth == 0) {
        return eval();
    }

    // should use a VLA (uint8_t scores[moves.size]) but removed in C++
    uint8_t scores[max_moves];

    for (uint32_t i = 0; i < moves.size; i++) {
        move_t move = moves[i];
        piece_t piece_start = board.get_piece(move.from).piece;
        piece_t piece_end = board.get_piece(move.to).piece;
        scores[i] = data::mvv_lva[piece_end][piece_start];
    }
    
    // search transposition table entry first (if it exists)
    if (entry_valid) {
        scores[entry.data.move_idx] = transposition_table_move_score;
    }

    int32_t best_eval = eval_min;
    uint8_t move_idx = 0;
    
    int32_t original_alpha = alpha;

    for (uint32_t i = 0; i < moves.size; i++, nodes++) {

        move_t move = order_moves(moves, scores, i);
        
        board.make_move(move);
        int32_t move_eval = -negamax(depth - 1, max_nodes, false, -beta, -alpha);
        board.undo_move(move);

        if (move_eval > best_eval) {
            best_eval = move_eval;
            if (root) {
                best_move = move;
                move_idx = i;
            }
        }

        if (!searching || nodes >= max_nodes) {
            return best_eval;
        }

        alpha = std::max(alpha, move_eval);
        if (alpha >= beta) {
            break; // beta cutoff
        }
    }
    transposition_table.store(best_eval, move_idx, zobrist_key, original_alpha, beta, depth);
    return best_eval;
}

int32_t chess_t::search(uint32_t max_depth, uint64_t max_nodes, bool use_opening_book) {
    searching = true;
    nodes = 0;
    if (use_opening_book && board.game_state_stack.size < 10) {
        if (opening_book.lookup(board, best_move)) {
            print_uci("info depth 0 nodes 0 score cp 0 time 0 nps 0 multipv 1 pv ");
            best_move.print();
            best_move.print(log);
            print_uci("\n");
            stop_search();
            return 0;
        }
    }
    // TODO: use partial search results
    int32_t eval;
    for (uint32_t depth = 1; depth < max_depth; depth++) {
        move_t old_best_move = best_move;
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        eval = negamax(depth, max_nodes);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        if (!searching || nodes >= max_nodes) {
            best_move = old_best_move;
            break;
        }
        std::chrono::duration<float> time = end - start;

        print_uci("info depth %u nodes %llu score cp %d time %lli nps %llu multipv 1 pv ",
                   depth, nodes, eval, std::chrono::duration_cast<std::chrono::milliseconds>(time).count(), (uint64_t)(nodes / time.count()));
        best_move.print();
        best_move.print(log);
        print_uci("\n");
    }
    stop_search();
    return eval;

}

int32_t chess_t::search_timed(std::chrono::milliseconds time, uint32_t max_depth, uint64_t max_nodes, bool use_opening_book) {
    std::future<int32_t> eval = std::async(std::launch::async, &chess_t::search, this, max_depth, max_nodes, use_opening_book);
    std::future_status status = eval.wait_for(time);
    if (status == std::future_status::timeout) {
        stop_search();
    }
    return eval.get();
}

void chess_t::stop_search() {
    searching = false;
}
