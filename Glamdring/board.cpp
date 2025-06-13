#include "chess.h"

void chess_t::board_t::print() {
    for (uint32_t i = 0; i < 8; i++) {
        for (uint32_t j = 0; j < 8; j++) {
            chess_t::square_t square = i * 8 + j;
            std::cout << (char)get_piece(square) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "Castling:\n";
    static constexpr char castling_names[2][2] = { { 'K', 'Q' }, { 'k', 'q' } };
    for (uint32_t color = 0; color < 2; color++) {
        for (uint32_t side = 0; side < 2; side++) {
            if (game_state_stack.last()->castling_rights[color][side]) {
                std::cout << castling_names[color][side];
            }
        }
    }
    std::cout << "\nEn Passant:\n";
    if (game_state_stack.last()->en_passant == null_square) {
        std::cout << "None";
    } else {
        char out[3];
        chess_t::square_to_file_rank(game_state_stack.last()->en_passant, out);
        std::cout << out;
    }
    std::cout << "\nHalf Move Clock:\n" << game_state_stack.last()->half_move_clock;
    std::cout << "\nFull Moves:\n" << game_state_stack.last()->full_moves;
    std::cout << '\n';
}

void chess_t::board_t::clear() {
    memset(board, CLEAR, sizeof(board));
    memset(bitboards, 0, sizeof(bitboards));
    memset(game_state_stack.data, 0, sizeof(game_state_stack.data));
    game_state_stack.size = 1;
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
        game_state_stack.last()->to_move = WHITE;
        break;
    case 'b':
        game_state_stack.last()->to_move = BLACK;
        break;
    }
    fen_idx += 2;
    for (; fen[fen_idx] != ' '; fen_idx++) {
        switch (fen[fen_idx]) {
        case 'K':
            game_state_stack.last()->castling_rights[WHITE][KINGSIDE] = true;
            break;
        case 'Q':
            game_state_stack.last()->castling_rights[WHITE][QUEENSIDE] = true;
            break;
        case 'k':
            game_state_stack.last()->castling_rights[BLACK][KINGSIDE] = true;
            break;
        case 'q':
            game_state_stack.last()->castling_rights[WHITE][QUEENSIDE] = true;
            break;
        }
    }
    fen_idx++;
    if (fen[fen_idx] == '-') {
        game_state_stack.last()->en_passant = null_square;
    } else {
        char file = fen[fen_idx];
        fen_idx++;
        char rank = fen[fen_idx];
        game_state_stack.last()->en_passant = file_rank_to_square(file, rank);
    }
    fen_idx++;
    std::sscanf(&fen[fen_idx], "%n %n", &game_state_stack.last()->half_move_clock, &game_state_stack.last()->full_moves);
}