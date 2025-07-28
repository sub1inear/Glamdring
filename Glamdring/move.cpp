#include "chess.h"
#include "data.h"

chess_t::move_t::move_t(board_t &board, const char *str) {
    from = file_rank_to_square(str[0], str[1]);
    to = file_rank_to_square(str[2], str[3]);
    compute_flags(board, str[4] == '\0' ? QUIET : (move_flags_t)(char_to_piece(str[4]) - 1), data::king_castling_end_squares);
}

chess_t::move_t::move_t(board_t &board, uint16_t polyglot_move) {    
    uint16_t to_file = polyglot_move & 0x7;
    uint16_t to_rank = (polyglot_move >> 3) & 0x7;
    uint16_t from_file = (polyglot_move >> 6) & 0x7;
    uint16_t from_rank = (polyglot_move >> 9) & 0x7;
    uint16_t promotion = polyglot_move >> 12;
    
    from = (7 - from_rank) * 8 + from_file;
    to = (7 - to_rank) * 8 + to_file;

    compute_flags(board, promotion ? (move_flags_t)(PROMOTION + promotion) : QUIET, data::king_castling_end_squares_polyglot);
    if (is_castling()) {
        to = data::king_castling_end_squares[board.game_state_stack.last()->to_move][flags - KING_CASTLE];
    }
}

void chess_t::move_t::compute_flags(board_t &board, move_flags_t promotion, const chess_t::square_t (&king_castling_end_squares)[][2]) {
    if (to == board.game_state_stack.last()->en_passant) {
        flags = EN_PASSANT_CAPTURE;    
    } else {
        flags = QUIET;
        piece_t start_piece = board.get_piece(from).piece;
        if (start_piece == KING) {
            color_t to_move = board.game_state_stack.last()->to_move;
            if (from == data::king_castling_start_squares[to_move]) {
                if (to == king_castling_end_squares[to_move][KINGSIDE]) {
                    flags = KING_CASTLE;
                } else if (to == king_castling_end_squares[to_move][QUEENSIDE]) {
                    flags = QUEEN_CASTLE;
                }
            }
        }
        if (start_piece == PAWN) {
            if (abs(from - to) == 16) {
                flags = DOUBLE_PAWN_PUSH;
            }
        }
        if (board.get_piece(to).piece != CLEAR) {
            flags = (move_flags_t)(flags | CAPTURE);
        }
        flags = (move_flags_t)(flags | promotion);
    }
}

void chess_t::move_t::print(FILE *out) {
    print_square(from, out);
    print_square(to, out);
    if (is_promotion()) {
        putc(piece_to_char(get_promotion()), out);
    }
}