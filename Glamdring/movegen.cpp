#include "chess.h"
#include "data.h"

inline void chess_t::serialize_bitboard(square_t square, uint64_t moves_bitboard, uint64_t enemies, move_array_t &moves) {
    for ( ; moves_bitboard; moves_bitboard = _blsr_u64(moves_bitboard)) { // clear lsb
         chess_t::square_t end_square = _tzcnt_u64(moves_bitboard); // count trailing zeros
        uint64_t lsb = _blsi_u64(moves_bitboard); // extract lsb
        move_t::move_flags_t flags = lsb & enemies ? move_t::CAPTURE : move_t::QUIET;
        moves.add({square, end_square, flags});
    }
}

void chess_t::gen_pawn_moves(uint64_t pawns, uint64_t blockers, uint64_t enemies, move_array_t &moves) {
    if (board.game_state_stack.last()->to_move == WHITE) {
        uint64_t single_move = pawns >> 8 & ~blockers;
        for (uint64_t moves_bitboard = single_move; moves_bitboard; moves_bitboard = _blsr_u64(moves_bitboard)) {
            chess_t::square_t end_square = _tzcnt_u64(moves_bitboard);
            chess_t::square_t start_square = end_square + 8;
            if (end_square < 8) {
                for (uint32_t f = move_t::KNIGHT_PROMOTION; f <= move_t::QUEEN_PROMOTION; f++) {
                    moves.add({start_square, end_square, (move_t::move_flags_t)f});
                }
            } else {
                moves.add({start_square, end_square, move_t::QUIET});
            }
        }
        constexpr uint64_t rank_4 = 0xff00000000;
        uint64_t double_move = single_move >> 8 & ~blockers & rank_4;
        for ( ; double_move; double_move = _blsr_u64(double_move)) {
            chess_t::square_t end_square = _tzcnt_u64(double_move);
            chess_t::square_t start_square = end_square + 16;
            moves.add({start_square, end_square, move_t::DOUBLE_PAWN_PUSH});
        }
        constexpr uint64_t file_a = 0x101010101010101;
        uint64_t capture_left_move = (pawns & ~file_a) >> 9;
        for (uint64_t moves_bitboard = capture_left_move & enemies; moves_bitboard; moves_bitboard = _tzcnt_u64(moves_bitboard)) {
            chess_t::square_t end_square = _tzcnt_u64(moves_bitboard);
            chess_t::square_t start_square = end_square + 9;
            if (end_square < 8) {
                for (uint32_t f = move_t::KNIGHT_PROMOTION_CAPTURE; f <= move_t::QUEEN_PROMOTION_CAPTURE; f++) {
                    moves.add({start_square, end_square, (move_t::move_flags_t)f});
                }
            } else {
                moves.add({start_square, end_square, move_t::CAPTURE});
            }
        }
        constexpr uint64_t file_h = 0x8080808080808080;
        uint64_t capture_right_move = (pawns & ~file_h) >> 7;
        for (uint64_t moves_bitboard = capture_right_move & enemies; moves_bitboard; moves_bitboard = _tzcnt_u64(moves_bitboard)) {
            chess_t::square_t end_square = _tzcnt_u64(moves_bitboard);
            chess_t::square_t start_square = end_square + 7;
            if (end_square < 8) {
                for (uint32_t f = move_t::KNIGHT_PROMOTION_CAPTURE; f <= move_t::QUEEN_PROMOTION_CAPTURE; f++) {
                    moves.add({start_square, end_square, (move_t::move_flags_t)f});
                }
            } else {
                moves.add({start_square, end_square, move_t::CAPTURE});
            }
        }
        chess_t::square_t en_passant = board.game_state_stack.last()->en_passant;
        if (en_passant != null_square) {
            uint64_t en_passant_bitboard = 1ull << en_passant;
            if (capture_left_move & en_passant_bitboard) {
                chess_t::square_t start_square = en_passant + 9;
                moves.add({start_square, en_passant, move_t::EN_PASSANT_CAPTURE});
            }
            if (capture_right_move & en_passant_bitboard) {
                chess_t::square_t start_square = en_passant + 7;
                moves.add({start_square, en_passant, move_t::EN_PASSANT_CAPTURE});
            }
        }
    } else {
        uint64_t single_move = pawns << 8 & ~blockers;
        for (uint64_t moves_bitboard = single_move; moves_bitboard; moves_bitboard = _tzcnt_u64(moves_bitboard)) {
            chess_t::square_t end_square = _tzcnt_u64(moves_bitboard);
            chess_t::square_t start_square = end_square - 8;
            if (end_square > 55) {
                for (uint32_t f = move_t::KNIGHT_PROMOTION; f <= move_t::QUEEN_PROMOTION; f++) {
                    moves.add({start_square, end_square, (move_t::move_flags_t)f});
                }
            } else {
                moves.add({start_square, end_square, move_t::QUIET});
            }
        }
        constexpr uint64_t rank_5 = 0xff000000;
        uint64_t double_move = single_move << 8 & ~blockers & rank_5;
        for ( ; double_move; double_move = _blsr_u64(double_move)) {
            chess_t::square_t end_square = _tzcnt_u64(double_move);
            chess_t::square_t start_square = end_square - 16;
            moves.add({start_square, end_square, move_t::DOUBLE_PAWN_PUSH});
        }
        constexpr uint64_t file_h = 0x8080808080808080;
        uint64_t capture_left_move = (pawns & ~file_h) << 9;
        for (uint64_t moves_bitboard = capture_left_move & enemies; moves_bitboard; moves_bitboard = _tzcnt_u64(moves_bitboard)) {
            chess_t::square_t end_square = _tzcnt_u64(moves_bitboard);
            chess_t::square_t start_square = end_square - 9;
            if (end_square > 55) {
                for (uint32_t f = move_t::KNIGHT_PROMOTION_CAPTURE; f <= move_t::QUEEN_PROMOTION_CAPTURE; f++) {
                    moves.add({start_square, end_square, (move_t::move_flags_t)f});
                }
            } else {
                moves.add({start_square, end_square, move_t::CAPTURE});
            }
        }
        constexpr uint64_t file_a = 0x101010101010101;
        uint64_t capture_right_move = (pawns & ~file_a) << 7;
        for (uint64_t moves_bitboard = capture_right_move & enemies; moves_bitboard; moves_bitboard = _blsr_u64(moves_bitboard)) {
            chess_t::square_t end_square = _tzcnt_u64(moves_bitboard);
            chess_t::square_t start_square = end_square - 7;
            if (end_square > 55) {
                for (uint32_t f = move_t::KNIGHT_PROMOTION_CAPTURE; f <= move_t::QUEEN_PROMOTION_CAPTURE; f++) {
                    moves.add({start_square, end_square, (move_t::move_flags_t)f});
                }
            } else {
                moves.add({start_square, end_square, move_t::CAPTURE});
            }
        }
        chess_t::square_t en_passant = board.game_state_stack.last()->en_passant;
        if (en_passant != null_square) {
            uint64_t en_passant_bitboard = 1ull << en_passant;
            if (capture_left_move & en_passant_bitboard) {
                chess_t::square_t start_square = en_passant - 9;
                moves.add({start_square, en_passant, move_t::EN_PASSANT_CAPTURE});
            }
            if (capture_right_move & en_passant_bitboard) {
                chess_t::square_t start_square = en_passant - 7;
                moves.add({start_square, en_passant, move_t::EN_PASSANT_CAPTURE});
            }
        }
    }
}
uint64_t chess_t::gen_knight_moves(square_t square, uint64_t allies) {
    uint64_t moves_bitboard = knight_move_data[square];
    moves_bitboard &= ~allies;
    return moves_bitboard;
}

uint64_t chess_t::gen_bishop_moves(square_t square, uint64_t blockers, uint64_t allies) {
    pext_t p = bishop_pext[square];
    uint64_t moves_bitboard = p.ptr[_pext_u64(blockers, p.mask)];
    moves_bitboard &= ~allies;
    return moves_bitboard;
}

uint64_t chess_t::gen_rook_moves(square_t square, uint64_t blockers, uint64_t allies) {
    pext_t p = rook_pext[square];
    uint64_t moves_bitboard = p.ptr[_pext_u64(blockers, p.mask)];
    moves_bitboard &= ~allies;
    return moves_bitboard;
}

uint64_t chess_t::gen_queen_moves(square_t square, uint64_t blockers, uint64_t allies) {
    pext_t bp = bishop_pext[square];
    uint64_t bishop_moves_bitboard = bp.ptr[_pext_u64(blockers, bp.mask)];
    pext_t rp = rook_pext[square];
    uint64_t rook_moves_bitboard = rp.ptr[_pext_u64(blockers, rp.mask)];
    uint64_t moves_bitboard = bishop_moves_bitboard | rook_moves_bitboard;
    moves_bitboard &= ~allies;
    return moves_bitboard;
}

uint64_t chess_t::gen_king_moves(square_t square, uint64_t allies) {
    uint64_t moves_bitboard = king_move_data[square];
    moves_bitboard &= ~allies;
    return moves_bitboard;
};

uint64_t chess_t::gen_blockers() {
    uint64_t blockers = board.bitboards[WHITE][PAWN] | board.bitboards[WHITE][KNIGHT] | board.bitboards[WHITE][BISHOP] | board.bitboards[WHITE][ROOK] | board.bitboards[WHITE][QUEEN] | board.bitboards[WHITE][KING];
    blockers |= board.bitboards[BLACK][PAWN] | board.bitboards[BLACK][KNIGHT] | board.bitboards[BLACK][BISHOP] | board.bitboards[BLACK][ROOK] | board.bitboards[BLACK][QUEEN] | board.bitboards[BLACK][KING];
    return blockers;
}

uint64_t chess_t::gen_allies() {
    color_t to_move = board.game_state_stack.last()->to_move;
    return board.bitboards[to_move][PAWN] | board.bitboards[to_move][KNIGHT] | board.bitboards[to_move][BISHOP] | board.bitboards[to_move][ROOK] | board.bitboards[to_move][QUEEN] | board.bitboards[to_move][KING];
}

chess_t::move_array_t chess_t::gen_moves() {
    move_array_t moves;
    uint64_t blockers = gen_blockers();
    uint64_t allies = gen_allies();
    uint64_t enemies = blockers & ~allies;
    color_t to_move = board.game_state_stack.last()->to_move;
    gen_pawn_moves(board.bitboards[to_move][PAWN], blockers, enemies, moves);
    for (uint64_t knights = board.bitboards[to_move][KNIGHT]; knights; knights = _blsr_u64(knights)) {
        chess_t::square_t square = _tzcnt_u64(knights);
        uint64_t moves_bitboard = gen_knight_moves(square, allies);
        serialize_bitboard(square, moves_bitboard, enemies, moves);
    }
    for (uint64_t bishops = board.bitboards[to_move][BISHOP]; bishops; bishops = _blsr_u64(bishops)) {
        chess_t::square_t square = _tzcnt_u64(bishops);
        uint64_t moves_bitboard = gen_bishop_moves(square, blockers, allies);
        serialize_bitboard(square, moves_bitboard, enemies, moves);
    }
    for (uint64_t rooks = board.bitboards[to_move][ROOK]; rooks; rooks = _blsr_u64(rooks)) {
        chess_t::square_t square = _tzcnt_u64(rooks);
        uint64_t moves_bitboard = gen_rook_moves(square, blockers, allies);
        serialize_bitboard(square, moves_bitboard, enemies, moves);
    }
    for (uint64_t queens = board.bitboards[to_move][QUEEN]; queens; queens = _blsr_u64(queens)) {
        chess_t::square_t square = _tzcnt_u64(queens);
        uint64_t moves_bitboard = gen_queen_moves(square, blockers, allies);
        serialize_bitboard(square, moves_bitboard, enemies, moves);
    }
    // assumes one king
    {
        chess_t::square_t square = _tzcnt_u64(board.bitboards[to_move][KING]);
        gen_king_moves(square, allies);
    }
    return moves;
}