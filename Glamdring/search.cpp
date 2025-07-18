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

int32_t chess_t::search(int32_t depth, bool root, int32_t alpha, int32_t beta) {

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
    
    if (moves.size == 0) {
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

    for (uint32_t i = 0; i < moves.size; i++) {
        move_t move = order_moves(moves, scores, i);
        
        board.make_move(move);
        int32_t move_eval = -search(depth - 1, false, -beta, -alpha);
        board.undo_move(move);

        if (move_eval > best_eval) {
            best_eval = move_eval;
            if (root) {
                best_move = move;
                move_idx = i;
            }
        }
        alpha = std::max(alpha, move_eval);
        if (alpha >= beta) {
            break; // beta cutoff
        }
    }
    transposition_table.store(best_eval, move_idx, zobrist_key, original_alpha, beta, depth);
    return best_eval;
}