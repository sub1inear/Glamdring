#include "chess.h"
#include "data.h"

void chess_t::board_t::set_piece(square_t square, piece_color_t piece) {
    board[square] = piece;
    game_state_stack.last()->zobrist_key ^= data::zobrist_random_data.piece[piece.color][piece.piece][square];
    bitboards[piece.color][piece.piece] |= 1ull << square;
}

void chess_t::board_t::clear_piece_bitboard(square_t square, piece_color_t piece) {
    game_state_stack.last()->zobrist_key ^= data::zobrist_random_data.piece[piece.color][piece.piece][square];
    bitboards[piece.color][piece.piece] &= ~(1ull << square);
}

void chess_t::board_t::clear_piece(square_t square, piece_color_t piece) {
    board[square].piece = CLEAR;
    clear_piece_bitboard(square, piece);
}

uint64_t chess_t::board_t::get_polyglot_key() {
    /*
    The internal Zobrist key is not compatible with PolyGlot as it
    unconditionally adds the en passant random data for the en passant file for speed
    (even if a pawn cannot legally capture it).
    PolyGlot requires that a pawn must be next to the capturable pawn
    (even if it cannot legally capture it).
    The following code converts from the internal Zobrist key into the correct PolyGlot one
    to be able to use PolyGlot opening books (as converting once to use an opening book is less speed critical than hash updates).
    */

    uint64_t zobrist_key = game_state_stack.last()->zobrist_key;

    if (game_state_stack.last()->en_passant != null_square) {  
        color_t to_move = game_state_stack.last()->to_move;

        square_t offset = game_state_stack.last()->to_move == WHITE ? 8 : -8;
        uint64_t pawn = 1ull << (game_state_stack.last()->en_passant + offset);

        constexpr uint64_t file_a = 0x101010101010101ull;
        constexpr uint64_t file_h = 0x8080808080808080ull;

        uint64_t capturing_pawns = (pawn & ~file_a) << 1 | (pawn & ~file_h) >> 1;

        if (!(bitboards[to_move][PAWN] & capturing_pawns)) {
            // remove en passant random data from key
            zobrist_key ^= data::zobrist_random_data.en_passant[game_state_stack.last()->en_passant % 8];
        }
    }
    return zobrist_key;
}

void chess_t::board_t::print(FILE *out) {
    game_state_t *game_state = game_state_stack.last();
    for (uint32_t i = 0; i < 8; i++) {
        fprintf(out, "\x1b[38;5;240m" "%c" "\x1b[0m" " ", '8' - i);
        for (uint32_t j = 0; j < 8; j++) {
            chess_t::square_t square = i * 8 + j;
            fprintf(out, "%c ", (char)get_piece(square));
        }
        fputc('\n', out);
    }
    fprintf(out,
            "\x1b[38;5;240m"
            "a b c d e f g h"
            "\x1b[0m\n"
            "To Move:\n"
            "%s\n"
            "Castling:\n",
            game_state->to_move == WHITE ? "White" : "Black");
    static constexpr char castling_names[2][2] = { { 'K', 'Q' }, { 'k', 'q' } };
    for (uint32_t color = 0; color < 2; color++) {
        for (uint32_t side = 0; side < 2; side++) {
            if (game_state->castling_rights[color][side]) {
                fputc(castling_names[color][side], out);
            }
        }
    }
    
    if (!game_state->castling_rights[WHITE][KINGSIDE] && !game_state->castling_rights[WHITE][QUEENSIDE] &&
        !game_state->castling_rights[BLACK][KINGSIDE] && !game_state->castling_rights[BLACK][QUEENSIDE]) {
        fputs("None", out);
    }
    fputs("\nEn Passant:\n", out);
    if (game_state->en_passant == null_square) {
        fputs("None", out);
    } else {
        print_square(game_state->en_passant, out);
    }
    fprintf(out,
            "\nHalf Move Clock:\n"
            "%d\n"
            "Full Moves:\n"
            "%d\n"
            "Zobrist Key:\n"
            "%llx\n"
            "\n",
            game_state->half_move_clock,
            game_state->full_moves,
            get_polyglot_key());
}

void chess_t::board_t::clear() {
    for (uint32_t i = 0; i < sizeof(board) / sizeof(board[0]); i++) {
        board[i] = { CLEAR, WHITE };
    }
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
        game_state_stack.last()->zobrist_key ^= data::zobrist_random_data.to_move;
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
            game_state_stack.last()->zobrist_key ^= data::zobrist_random_data.castling[WHITE][KINGSIDE];
            break;
        case 'Q':
            game_state_stack.last()->castling_rights[WHITE][QUEENSIDE] = true;
            game_state_stack.last()->zobrist_key ^= data::zobrist_random_data.castling[WHITE][QUEENSIDE];
            break;
        case 'k':
            game_state_stack.last()->castling_rights[BLACK][KINGSIDE] = true;
            game_state_stack.last()->zobrist_key ^= data::zobrist_random_data.castling[BLACK][KINGSIDE];
            break;
        case 'q':
            game_state_stack.last()->castling_rights[BLACK][QUEENSIDE] = true;
            game_state_stack.last()->zobrist_key ^= data::zobrist_random_data.castling[BLACK][QUEENSIDE];
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
        game_state_stack.last()->zobrist_key ^= data::zobrist_random_data.en_passant[file - 'a'];
    }
    fen_idx++;
    sscanf(&fen[fen_idx], "%d %d", &game_state_stack.last()->half_move_clock, &game_state_stack.last()->full_moves);
}

void chess_t::board_t::make_move(move_t move) {
    game_state_t *old_game_state = game_state_stack.last();
    
    piece_color_t start_piece = get_piece(move.from);
    piece_color_t new_piece = start_piece;
    piece_color_t captured_piece = get_piece(move.to);

    // TODO: inline this in the two places used to wasting cycles when unneeded
    square_t new_en_passant = old_game_state->to_move == WHITE ? move.to + 8 : move.to - 8;

    if (move.is_promotion()) {
        new_piece = { move.get_promotion(), old_game_state->to_move };
    }

    set_piece(move.to, new_piece);

    if (move.flags == move_t::EN_PASSANT_CAPTURE) {
        captured_piece = get_piece(new_en_passant);
        clear_piece(new_en_passant, captured_piece);
    } else if (move.is_capture()) {
        clear_piece_bitboard(move.to, captured_piece);
    }
    
    clear_piece(move.from, start_piece);
    
    // TODO: check if size is greater than max_ply
    game_state_t *new_game_state = game_state_stack.next();

    new_game_state->zobrist_key = old_game_state->zobrist_key;

    // reset half move clock on captures and pawn moves, increase otherwise
    new_game_state->half_move_clock = move.is_capture() || start_piece.piece == PAWN ? 0 : old_game_state->half_move_clock + 1;

    new_game_state->full_moves = old_game_state->to_move == WHITE ? old_game_state->full_moves : old_game_state->full_moves + 1;

    if (old_game_state->en_passant != null_square) {
        new_game_state->zobrist_key ^= data::zobrist_random_data.en_passant[old_game_state->en_passant % 8];
    }
    if (move.flags == move_t::DOUBLE_PAWN_PUSH) {
        new_game_state->en_passant = new_en_passant;
        new_game_state->zobrist_key ^= data::zobrist_random_data.en_passant[new_game_state->en_passant % 8];
    } else {
        new_game_state->en_passant = null_square;
    }

    new_game_state->to_move = (color_t)!old_game_state->to_move;
    new_game_state->zobrist_key ^= data::zobrist_random_data.to_move;

    new_game_state->captured_piece = captured_piece;

    memcpy(new_game_state->castling_rights, old_game_state->castling_rights, sizeof(new_game_state->castling_rights));
    
    if (move.is_castling()) {        
        castling_side_t side = move.get_castling();
        
        square_t rook_start_square = data::rook_castling_start_squares[old_game_state->to_move][side];
        square_t rook_end_square = data::rook_castling_end_squares[old_game_state->to_move][side];

        piece_color_t rook = get_piece(rook_start_square);
        
        clear_piece(rook_start_square, rook);
        set_piece(rook_end_square, rook);
    }
    if (start_piece.piece == KING) {
        if (new_game_state->castling_rights[old_game_state->to_move][KINGSIDE]) {
            new_game_state->zobrist_key ^= data::zobrist_random_data.castling[old_game_state->to_move][KINGSIDE];
        }
         if (new_game_state->castling_rights[old_game_state->to_move][QUEENSIDE]) {
            new_game_state->zobrist_key ^= data::zobrist_random_data.castling[old_game_state->to_move][QUEENSIDE];
        }
        memset(&new_game_state->castling_rights[old_game_state->to_move], false, sizeof(new_game_state->castling_rights[old_game_state->to_move]));

    }

    if (start_piece.piece == ROOK) {
        if (move.from == data::rook_castling_start_squares[old_game_state->to_move][KINGSIDE]) {
            if (new_game_state->castling_rights[old_game_state->to_move][KINGSIDE]) {
                new_game_state->zobrist_key ^= data::zobrist_random_data.castling[old_game_state->to_move][KINGSIDE];
            }
            new_game_state->castling_rights[old_game_state->to_move][KINGSIDE] = false;
        } else if (move.from == data::rook_castling_start_squares[old_game_state->to_move][QUEENSIDE]) {
            if (new_game_state->castling_rights[old_game_state->to_move][QUEENSIDE]) {
                new_game_state->zobrist_key ^= data::zobrist_random_data.castling[old_game_state->to_move][QUEENSIDE];
            }
            new_game_state->castling_rights[old_game_state->to_move][QUEENSIDE] = false;
        }
    }
    if (captured_piece.piece == ROOK) {
        if (move.to == data::rook_castling_start_squares[new_game_state->to_move][KINGSIDE]) {
            if (new_game_state->castling_rights[new_game_state->to_move][KINGSIDE]) {
                new_game_state->zobrist_key ^= data::zobrist_random_data.castling[old_game_state->to_move][KINGSIDE];
            }
            new_game_state->castling_rights[new_game_state->to_move][KINGSIDE] = false;    
        } else if (move.to == data::rook_castling_start_squares[new_game_state->to_move][QUEENSIDE]) {
            if (new_game_state->castling_rights[new_game_state->to_move][QUEENSIDE]) {
                new_game_state->zobrist_key ^= data::zobrist_random_data.castling[old_game_state->to_move][QUEENSIDE];
            }
            new_game_state->castling_rights[new_game_state->to_move][QUEENSIDE] = false;
        }
    }
}

void chess_t::board_t::undo_move(move_t move) {
    game_state_t *old_game_state = game_state_stack.last();
    game_state_t *new_game_state = game_state_stack.pop();

    piece_color_t start_piece = get_piece(move.to);
    piece_color_t new_piece = start_piece;

    if (move.is_promotion()) {
        new_piece = { PAWN, new_game_state->to_move };
    }

    set_piece(move.from, new_piece);

    if (move.is_capture()) {
        if (move.flags == move_t::EN_PASSANT_CAPTURE) {
            clear_piece(move.to, start_piece); 
            set_piece(new_game_state->to_move == WHITE ? move.to + 8 : move.to - 8, old_game_state->captured_piece);
        } else {
            clear_piece_bitboard(move.to, start_piece);
            set_piece(move.to, old_game_state->captured_piece);
        }
    } else {
        clear_piece(move.to, start_piece);
    }

    if (move.is_castling()) {
        castling_side_t side = move.get_castling();
        
        square_t rook_start_square = data::rook_castling_start_squares[new_game_state->to_move][side];
        square_t rook_end_square = data::rook_castling_end_squares[new_game_state->to_move][side];
    
        piece_color_t rook = get_piece(rook_end_square);

        clear_piece(rook_end_square, rook);
        set_piece(rook_start_square, rook);
    }
}