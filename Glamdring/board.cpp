#include "chess.h"

void chess_t::board_t::print() {
    for (square_t square = 0; square < 64; square++) {
        std::cout << (char)get_piece(square) << ' ';
        if (square % 8 == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "Castling:\n";
    static constexpr char castling_names[2][2] = {'K', 'Q', 'k', 'q'};
    for (uint32_t color = 0; color < 2; color++) {
        for (uint32_t side = 0; side < 2; side++) {
            if (castling_rights[color][side]) {
                std::cout << castling_names[color][side];
            }
        }
    }
    std::cout << "\nEn Passant:\n";
    if (en_passant == null_square) {
        std::cout << "None";
    } else {
        char out[3];
        chess_t::square_to_file_rank(en_passant, out);
        std::cout << out;
    }
    std::cout << "\nHalf Move Clock:\n" << half_move_clock;
    std::cout << "\nFull Moves:\n" << full_moves;
}

void chess_t::board_t::clear() {
    memset(board, CLEAR, sizeof(board));
}

void chess_t::board_t::load_fen(const char *fen) {
    clear();
    uint32_t fen_idx = 0;
    for (square_t square = 0; square < sizeof(board) / sizeof(board[0]); fen_idx++) {
        char c = fen[fen_idx];
        if (isdigit(c)) {
            square += c - '0';
        } else if (c != '/') {
            set_piece(square, c);
            square++;
        }
    }
    fen_idx++;
    switch (fen[fen_idx]) {
    case 'w':
        set_to_move(WHITE);
        break;
    case 'b':
        set_to_move(BLACK);
        break;
    }
    fen_idx += 2;
    for (; fen[fen_idx] != ' '; fen_idx++) {
        switch (fen[fen_idx]) {
        case 'K':
            set_castling_rights(WHITE, KINGSIDE, true);
            break;
        case 'Q':
            set_castling_rights(WHITE, QUEENSIDE, true);
            break;
        case 'k':
            set_castling_rights(BLACK, KINGSIDE, true);
            break;
        case 'q':
            set_castling_rights(BLACK, QUEENSIDE, true);
            break;
        }
    }
    fen_idx++;
    if (fen[fen_idx] == '-') {
        set_en_passant(null_square);
    } else {
        char file = fen[fen_idx];
        fen_idx++;
        char rank = fen[fen_idx];
        set_en_passant(file_rank_to_square(file, rank));
    }
    fen_idx++;
    std::sscanf(&fen[fen_idx], "%n %n", &half_move_clock, &full_moves);
}