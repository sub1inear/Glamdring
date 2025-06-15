#include "chess.h"
#include "data.h"

inline void chess_t::serialize_bitboard(square_t square, uint64_t moves_bitboard, uint64_t enemies, move_array_t &moves) {
    for ( ; moves_bitboard; moves_bitboard = _blsr_u64(moves_bitboard)) { // clear lsb
        chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard); // count trailing zeros
        uint64_t lsb = _blsi_u64(moves_bitboard); // extract lsb
        move_t::move_flags_t flags = lsb & enemies ? move_t::CAPTURE : move_t::QUIET;
        moves.add({square, end_square, flags});
    }
}

void chess_t::gen_pawn_moves(uint64_t pawns, uint64_t blockers, uint64_t enemies, move_array_t &moves) {
    if (board.game_state_stack.last()->to_move == WHITE) {
        uint64_t single_move = pawns >> 8 & ~blockers;
        for (uint64_t moves_bitboard = single_move; moves_bitboard; moves_bitboard = _blsr_u64(moves_bitboard)) {
            chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard);
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
            chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(double_move);
            chess_t::square_t start_square = end_square + 16;
            moves.add({start_square, end_square, move_t::DOUBLE_PAWN_PUSH});
        }
        constexpr uint64_t file_a = 0x101010101010101;
        uint64_t capture_left_move = (pawns & ~file_a) >> 9;
        for (uint64_t moves_bitboard = capture_left_move & enemies; moves_bitboard; moves_bitboard = _tzcnt_u64(moves_bitboard)) {
            chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard);
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
            chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard);
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
            chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard);
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
            chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(double_move);
            chess_t::square_t start_square = end_square - 16;
            moves.add({start_square, end_square, move_t::DOUBLE_PAWN_PUSH});
        }
        constexpr uint64_t file_h = 0x8080808080808080;
        uint64_t capture_left_move = (pawns & ~file_h) << 9;
        for (uint64_t moves_bitboard = capture_left_move & enemies; moves_bitboard; moves_bitboard = _tzcnt_u64(moves_bitboard)) {
            chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard);
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
            chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard);
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
uint64_t chess_t::gen_pawn_attacks(color_t to_move, square_t square) {
    return data::pawn_attack_data[to_move][square];
}
uint64_t chess_t::gen_knight_moves(square_t square, uint64_t allies) {
    uint64_t moves_bitboard = data::knight_move_data[square];
    moves_bitboard &= ~allies;
    return moves_bitboard;
}

uint64_t chess_t::gen_bishop_moves(square_t square, uint64_t blockers, uint64_t allies) {
    data::pext_t p = data::bishop_pext[square];
    uint64_t moves_bitboard = p.ptr[_pext_u64(blockers, p.mask)];
    moves_bitboard &= ~allies;
    return moves_bitboard;
}

uint64_t chess_t::gen_rook_moves(square_t square, uint64_t blockers, uint64_t allies) {
    data::pext_t p = data::rook_pext[square];
    uint64_t moves_bitboard = p.ptr[_pext_u64(blockers, p.mask)];
    moves_bitboard &= ~allies;
    return moves_bitboard;
}

uint64_t chess_t::gen_queen_moves(square_t square, uint64_t blockers, uint64_t allies) {
    data::pext_t bp = data::bishop_pext[square];
    uint64_t bishop_moves_bitboard = bp.ptr[_pext_u64(blockers, bp.mask)];
    data::pext_t rp = data::rook_pext[square];
    uint64_t rook_moves_bitboard = rp.ptr[_pext_u64(blockers, rp.mask)];
    uint64_t moves_bitboard = bishop_moves_bitboard | rook_moves_bitboard;
    moves_bitboard &= ~allies;
    return moves_bitboard;
}

uint64_t chess_t::gen_king_moves(square_t square, uint64_t allies) {
    uint64_t moves_bitboard = data::king_move_data[square];
    moves_bitboard &= ~allies;
    return moves_bitboard;
};

void chess_t::gen_castling_moves(square_t square, uint64_t blockers, move_array_t &moves) {
    color_t to_move = board.game_state_stack.last()->to_move;
    if (!(blockers & data::king_castling_data[to_move][KINGSIDE]) &&
        board.game_state_stack.last()->castling_rights[to_move][KINGSIDE]) {
        moves.add({ square, data::king_castling_end_squares[to_move][KINGSIDE], move_t::KING_CASTLE }); 
    }
    if (!(blockers & data::king_castling_data[to_move][QUEENSIDE]) &&
        board.game_state_stack.last()->castling_rights[to_move][QUEENSIDE]) {
        moves.add({ square, data::king_castling_end_squares[to_move][QUEENSIDE], move_t::QUEEN_CASTLE }); 
    }
}

uint64_t chess_t::gen_blockers() {
    uint64_t blockers = board.bitboards[WHITE][PAWN] | board.bitboards[WHITE][KNIGHT] | board.bitboards[WHITE][BISHOP] | board.bitboards[WHITE][ROOK] | board.bitboards[WHITE][QUEEN] | board.bitboards[WHITE][KING];
    blockers |= board.bitboards[BLACK][PAWN] | board.bitboards[BLACK][KNIGHT] | board.bitboards[BLACK][BISHOP] | board.bitboards[BLACK][ROOK] | board.bitboards[BLACK][QUEEN] | board.bitboards[BLACK][KING];
    return blockers;
}

uint64_t chess_t::gen_allies() {
    color_t to_move = board.game_state_stack.last()->to_move;
    return board.bitboards[to_move][PAWN] | board.bitboards[to_move][KNIGHT] | board.bitboards[to_move][BISHOP] | board.bitboards[to_move][ROOK] | board.bitboards[to_move][QUEEN] | board.bitboards[to_move][KING];
}

chess_t::attack_data_t chess_t::gen_attackers(square_t square, uint64_t blockers) {
    color_t to_move = board.game_state_stack.last()->to_move;
    color_t other_to_move = (color_t)!to_move;
    uint64_t attackers = 0;
    uint64_t attacking = 0;
    // generate moves attacking square by anding moves from square and moves to square
    // enemy's allies unused  as only taking moves that intersect with enemy
    attackers |= gen_pawn_attacks(to_move, square) & board.bitboards[other_to_move][PAWN];
    attackers |= gen_knight_moves(square, 0ull) & board.bitboards[other_to_move][KNIGHT];
    
    uint64_t bishop_moves_from_square = gen_bishop_moves(square, blockers, 0ull);
    uint64_t bishop_attackers = bishop_moves_from_square & board.bitboards[other_to_move][BISHOP];
    attackers |= bishop_attackers;
    
    for ( ; bishop_attackers; bishop_attackers = _blsr_u64(bishop_attackers)) {
        chess_t::square_t bishop_square = (chess_t::square_t)_tzcnt_u64(bishop_attackers);
        attacking |= gen_bishop_moves(bishop_square, blockers, 0ull) & bishop_moves_from_square;
    }
    
    uint64_t rook_moves_from_square = gen_rook_moves(square, blockers, 0ull);
    uint64_t rook_attackers = rook_moves_from_square & board.bitboards[other_to_move][ROOK];
    attackers |= rook_attackers;
    
    for ( ; rook_attackers; rook_attackers = _blsr_u64(rook_attackers)) {
        chess_t::square_t rook_square = (chess_t::square_t)_tzcnt_u64(rook_attackers);
        attacking |= gen_rook_moves(rook_square, blockers, 0ull) & rook_moves_from_square;
    }

    uint64_t queen_moves_from_square = bishop_moves_from_square | rook_moves_from_square;
    uint64_t queen_attackers = queen_moves_from_square & board.bitboards[other_to_move][QUEEN];
    attackers |= queen_attackers;
    
    for ( ; queen_attackers; queen_attackers = _blsr_u64(queen_attackers)) {
        chess_t::square_t queen_square = (chess_t::square_t)_tzcnt_u64(queen_attackers);
        attacking |= gen_queen_moves(queen_square, blockers, 0ull) & queen_moves_from_square;
    }

    attackers |= gen_king_moves(square, 0ull) & board.bitboards[other_to_move][KING];
    return { attackers, attackers | attacking };
}

uint64_t chess_t::gen_king_danger_squares(uint64_t blockers) {
    uint64_t attacked = 0;
    color_t to_move = board.game_state_stack.last()->to_move;
    color_t other_to_move = (color_t)!to_move;

    // xray through king (bitboard must ensure king can't move backward out of check)
    uint64_t king_bitboard = board.bitboards[to_move][KING];
    board.bitboards[to_move][KING] = 0;

    // assumes one king
    {
        chess_t::square_t king_square = (chess_t::square_t)_tzcnt_u64(board.bitboards[other_to_move][KING]);
        attacked |= gen_king_moves(king_square, 0ull);
    }
    for (uint64_t pawns = board.bitboards[other_to_move][PAWN]; pawns; pawns = _blsr_u64(pawns)) {
        chess_t::square_t pawn_square = (chess_t::square_t)_tzcnt_u64(pawns);
        attacked |= gen_pawn_attacks(other_to_move, pawn_square);
    }
    for (uint64_t knights = board.bitboards[other_to_move][KNIGHT]; knights; knights = _blsr_u64(knights)) {
        chess_t::square_t knight_square = (chess_t::square_t)_tzcnt_u64(knights);
        attacked |= gen_knight_moves(knight_square, 0ull);
    }
    for (uint64_t bishops = board.bitboards[other_to_move][BISHOP]; bishops; bishops = _blsr_u64(bishops)) {
        chess_t::square_t bishop_square = (chess_t::square_t)_tzcnt_u64(bishops);
        attacked |= gen_bishop_moves(bishop_square, blockers, 0ull);
    }
    for (uint64_t rooks = board.bitboards[other_to_move][ROOK]; rooks; rooks = _blsr_u64(rooks)) {
        chess_t::square_t rook_square = (chess_t::square_t)_tzcnt_u64(rooks);
        attacked |= gen_rook_moves(rook_square, blockers, 0ull);
    }
    for (uint64_t queens = board.bitboards[other_to_move][QUEEN]; queens; queens = _blsr_u64(queens)) {
        chess_t::square_t queen_square = (chess_t::square_t)_tzcnt_u64(queens);
        attacked |= gen_queen_moves(queen_square, blockers, 0ull);
    }
    board.bitboards[to_move][KING] = king_bitboard;
    return attacked;
}

uint64_t chess_t::gen_pins(square_t square, uint64_t danger, uint64_t allies, uint64_t enemies) {
    allies &= ~danger;
    uint64_t blockers = allies | enemies;
    print_bitboard(blockers);
    return gen_attackers(square, blockers).attack_mask;
}

chess_t::move_array_t chess_t::gen_moves() {
    move_array_t moves;
    
    color_t to_move = board.game_state_stack.last()->to_move;

    uint64_t blockers = gen_blockers();
    uint64_t allies = gen_allies();
    uint64_t enemies = blockers & ~allies;

    chess_t::square_t king_square = (chess_t::square_t)_tzcnt_u64(board.bitboards[to_move][KING]);

    attack_data_t check_data = gen_attackers(king_square, blockers);
    uint32_t num_checkers = (uint32_t)_mm_popcnt_u64(check_data.attackers);
    uint64_t danger = gen_king_danger_squares(blockers);
    uint64_t pins = gen_pins(king_square, danger, allies, enemies);

    // assumes one king
    {
        uint64_t moves_bitboard = gen_king_moves(king_square, allies);
        serialize_bitboard(king_square, moves_bitboard, enemies, moves);
        gen_castling_moves(king_square, blockers, moves);
    }

    gen_pawn_moves(board.bitboards[to_move][PAWN], blockers, enemies, moves);
    for (uint64_t knights = board.bitboards[to_move][KNIGHT]; knights; knights = _blsr_u64(knights)) {
        chess_t::square_t knight_square = (chess_t::square_t)_tzcnt_u64(knights);
        uint64_t moves_bitboard = gen_knight_moves(knight_square, allies);
        serialize_bitboard(knight_square, moves_bitboard, enemies, moves);
    }
    for (uint64_t bishops = board.bitboards[to_move][BISHOP]; bishops; bishops = _blsr_u64(bishops)) {
        chess_t::square_t bishop_square = (chess_t::square_t)_tzcnt_u64(bishops);
        uint64_t moves_bitboard = gen_bishop_moves(bishop_square, blockers, allies);
        serialize_bitboard(bishop_square, moves_bitboard, enemies, moves);
    }
    for (uint64_t rooks = board.bitboards[to_move][ROOK]; rooks; rooks = _blsr_u64(rooks)) {
        chess_t::square_t rook_square = (chess_t::square_t)_tzcnt_u64(rooks);
        uint64_t moves_bitboard = gen_rook_moves(rook_square, blockers, allies);
        serialize_bitboard(rook_square, moves_bitboard, enemies, moves);
    }
    for (uint64_t queens = board.bitboards[to_move][QUEEN]; queens; queens = _blsr_u64(queens)) {
        chess_t::square_t queen_square = (chess_t::square_t)_tzcnt_u64(queens);
        uint64_t moves_bitboard = gen_queen_moves(queen_square, blockers, allies);
        serialize_bitboard(queen_square, moves_bitboard, enemies, moves);
    }
    return moves;
}