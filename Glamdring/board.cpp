#include "chess.h"
#include "data.h"

void chess_t::board_t::print() {
    game_state_t *game_state = game_state_stack.last();
    for (uint32_t i = 0; i < 8; i++) {
        std::cout << "\x1b[38;5;240m" <<  (char)('8' - i) << " \033[0m";
        for (uint32_t j = 0; j < 8; j++) {
            chess_t::square_t square = i * 8 + j;
            std::cout << (char)get_piece(square) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "\x1b[38;5;240m  a b c d e f g h\033[0m\nTo Move:\n";
    std::cout << (game_state->to_move == WHITE ? "White" : "Black");
    std::cout << "\nCastling:\n";
    static constexpr char castling_names[2][2] = { { 'K', 'Q' }, { 'k', 'q' } };
    for (uint32_t color = 0; color < 2; color++) {
        for (uint32_t side = 0; side < 2; side++) {
            if (game_state->castling_rights[color][side]) {
                std::cout << castling_names[color][side];
            }
        }
    }
    
    if (!game_state->castling_rights[WHITE][KINGSIDE] && !game_state->castling_rights[WHITE][QUEENSIDE] &&
        !game_state->castling_rights[BLACK][KINGSIDE] && !game_state->castling_rights[BLACK][QUEENSIDE]) {
        std::cout << "None";
    }
    std::cout << "\nEn Passant:\n";
    if (game_state->en_passant == null_square) {
        std::cout << "None";
    } else {
        print_square(game_state->en_passant);
    }
    std::cout << "\nHalf Move Clock:\n" << game_state->half_move_clock;
    std::cout << "\nFull Moves:\n" << game_state->full_moves;
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
            game_state_stack.last()->castling_rights[BLACK][QUEENSIDE] = true;
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

void chess_t::board_t::make_move(move_t move) {
    game_state_t *old_game_state = game_state_stack.last();
    game_state_t *new_game_state = game_state_stack.next();
    
    piece_color_t start_piece = get_piece(move.from);
    piece_color_t new_piece = start_piece;
    piece_color_t captured_piece = get_piece(move.to);

    square_t new_en_passant = old_game_state->to_move == WHITE ? move.to + 8 : move.to - 8;

    if (move.is_promotion()) {
        new_piece = { move.get_promotion(), old_game_state->to_move };
    }

    set_piece(move.to, start_piece);

    if (move.flags == move_t::EN_PASSANT_CAPTURE) {
        captured_piece = get_piece(new_en_passant);
        clear_piece(new_en_passant, captured_piece);
    }
    
    clear_piece(move.from, start_piece);
    
    // reset half move clock on captures and pawn moves, reset otherwise
    new_game_state->half_move_clock = move.is_capture() || start_piece.piece == PAWN ? 0 : old_game_state->half_move_clock + 1;
    new_game_state->full_moves = old_game_state->to_move == WHITE ? old_game_state->full_moves : old_game_state->full_moves + 1;
    new_game_state->en_passant = move.flags == move_t::DOUBLE_PAWN_PUSH ? new_en_passant : null_square;
    new_game_state->to_move = (color_t)!old_game_state->to_move;
    new_game_state->captured_piece = captured_piece;

    memcpy(new_game_state->castling_rights, old_game_state->castling_rights, sizeof(new_game_state->castling_rights));
    if (move.is_castling()) {
        memset(&new_game_state->castling_rights[old_game_state->to_move], false, sizeof(new_game_state->castling_rights[old_game_state->to_move]));
        
        castling_side_t side = move.get_castling();
        
        square_t rook_start_square = data::rook_castling_start_squares[old_game_state->to_move][side];
        square_t rook_end_square = data::rook_castling_end_squares[old_game_state->to_move][side];

        piece_color_t rook = get_piece(rook_start_square);
        
        clear_piece(rook_start_square, rook);
        set_piece(rook_end_square, rook);
    } else if (start_piece.piece == KING) {
        // TODO: use & for branchless clear
        memset(&new_game_state->castling_rights[old_game_state->to_move], false, sizeof(new_game_state->castling_rights[old_game_state->to_move]));
    }
    if (start_piece.piece == ROOK) {
        // TODO: use & for branchless clear
        memset(&new_game_state->castling_rights[old_game_state->to_move], false, sizeof(new_game_state->castling_rights[old_game_state->to_move]));
    }
   

}

void chess_t::board_t::undo_move(move_t move) {
    game_state_t *game_state = game_state_stack.last();

    piece_color_t start_piece = get_piece(move.to);

    set_piece(move.from, start_piece);
    set_piece(move.to, game_state->captured_piece);

    game_state_stack.pop();
}