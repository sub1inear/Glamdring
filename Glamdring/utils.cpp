#include "chess.h"
#include "data.h"

char chess_t::piece_to_char(piece_t piece) {
    return data::piece_to_char[BLACK][piece];
}

chess_t::piece_t chess_t::char_to_piece(char c) {
    for (uint32_t piece_idx = 0; piece_idx < 6; piece_idx++) {
        if (c == data::piece_to_char[BLACK][piece_idx]) {
            return (chess_t::piece_t)piece_idx;
        }
    }
    return CLEAR;
}

chess_t::piece_color_t::piece_color_t(char c) {
    for (uint32_t color_idx = 0; color_idx < 2; color_idx++) {
        for (uint32_t piece_idx = 0; piece_idx < 6; piece_idx++) {
            if (c == data::piece_to_char[color_idx][piece_idx]) {
                color = (color_t)color_idx;
                piece = (piece_t)piece_idx;
            }
        }
    }
}

chess_t::piece_color_t::operator char() {
    return data::piece_to_char[color][piece];
}

chess_t::square_t chess_t::file_rank_to_square(square_t file, square_t rank) {
    square_t square = ('8' - rank) * 8;
    square += file - 'a';
    return square;
}

void chess_t::square_to_file_rank(square_t square, char *out) {
    out[0] = 'a' + square % 8;
    out[1] = '8' - square / 8;
    out[2] = '\0';
}

void chess_t::print_square(square_t square, FILE *out) {
    putc('a' + square % 8, out);
    putc('8' - square / 8, out);
}
 
void chess_t::print_bitboard(uint64_t bitboard) {
    for (uint32_t i = 0; i < 8; i++) {
        for (uint32_t j = 0; j < 8; j++) {
            putc('0' + ((bitboard >> (i * 8 + j)) & 0x1), stdout);
            putc(' ', stdout);
        }
        putc('\n', stdout);
    }
    putc('\n', stdout);
}

chess_t::move_t::move_t(board_t &board, const char *str) {
    from = file_rank_to_square(str[0], str[1]);
    to = file_rank_to_square(str[2], str[3]);
    if (to == board.game_state_stack.last()->en_passant) {
        flags = EN_PASSANT_CAPTURE;    
    } else {
        flags = QUIET;
        piece_t start_piece = board.get_piece(from).piece;
        if (start_piece == KING) {
            color_t to_move = board.game_state_stack.last()->to_move;
            if (from == data::king_castling_start_squares[to_move]) {
                if (to == data::king_castling_end_squares[to_move][KINGSIDE]) {
                    flags = KING_CASTLE;
                } else if (to == data::king_castling_end_squares[to_move][QUEENSIDE]) {
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
        if (str[4] != '\0') {
            flags = (move_flags_t)(flags | PROMOTION | (char_to_piece(str[4]) - 1));
        }
    }
}