#include "chess.h"
#include "data.h"

chess_t::score_array_t chess_t::score_moves(move_array_t &moves) {
    score_array_t scores(moves.size);
    for (uint32_t i = 0; i < moves.size; i++) {
        move_t move = moves[i];
        piece_t piece_start = board.get_piece(move.from).piece;
        piece_t piece_end = board.get_piece(move.to).piece;
        scores[i] = data::mvv_lva[piece_end][piece_start];
    }
    return scores;
}
chess_t::move_t chess_t::order_moves(move_array_t &moves, score_array_t &scores, uint32_t i) {
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
    move_array_t moves = gen_moves();
    if (moves.size == 0) {
        return eval_min;
    }
    if (depth == 0) {
        return eval();
    }

    score_array_t scores = score_moves(moves);

    int32_t best_eval = eval_min;

    for (uint32_t i = 0; i < moves.size; i++, nodes++) {
        move_t move = order_moves(moves, scores, i);
        
        board.make_move(move);
        int32_t move_eval = -search(depth - 1, false, -beta, -alpha);
        board.undo_move(move);

        if (move_eval > best_eval) {
            best_eval = move_eval;
            if (root) {
                best_move = move;
            }
        }
        alpha = std::max(alpha, move_eval);
        if (alpha >= beta) {
            break; // beta cutoff
        }
    }
    return best_eval;
}