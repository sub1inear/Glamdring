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

template <chess_t::color_t to_move> 
void chess_t::gen_pawn_moves(uint64_t pawns, uint64_t blockers, uint64_t allies, uint64_t enemies, uint64_t legal, uint64_t *pin_lines, move_array_t &moves) {
    constexpr color_t other_to_move = (color_t)!to_move;

    uint64_t single_move = (to_move == WHITE ? pawns >> 8 : pawns << 8) & ~blockers;
    for (uint64_t moves_bitboard = single_move & legal; moves_bitboard; moves_bitboard = _blsr_u64(moves_bitboard)) {
        chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard);
        chess_t::square_t start_square = end_square + (to_move == WHITE ? 8 : -8);
        if (1ull << end_square & ~pin_lines[start_square]) {
            continue;    
        }
        if (end_square < 8) {
            for (uint32_t f = move_t::KNIGHT_PROMOTION; f <= move_t::QUEEN_PROMOTION; f++) {
                moves.add({start_square, end_square, (move_t::move_flags_t)f});
            }
        } else {
            moves.add({start_square, end_square, move_t::QUIET});
        }
    }
    constexpr uint64_t rank_4 = 0xff00000000ull;
    constexpr uint64_t rank_5 = 0xff000000ull;
    constexpr uint64_t double_move_rank = to_move == WHITE ? rank_4 : rank_5;
    uint64_t double_move = (to_move == WHITE ? single_move >> 8 : single_move << 8) & ~blockers & double_move_rank & legal;
    for ( ; double_move; double_move = _blsr_u64(double_move)) {
        chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(double_move);
        chess_t::square_t start_square = end_square + (to_move == WHITE ? 16 : -16);
        if (1ull << end_square & ~pin_lines[start_square]) {
            continue;    
        }
        moves.add({start_square, end_square, move_t::DOUBLE_PAWN_PUSH});
    }
    constexpr uint64_t file_a = 0x101010101010101ull;
    constexpr uint64_t file_h = 0x8080808080808080ull;

    uint64_t capture_left_move = (to_move == WHITE ? (pawns & ~file_a) >> 9 : (pawns & ~file_h) << 9);
    for (uint64_t moves_bitboard = capture_left_move & enemies & legal; moves_bitboard; moves_bitboard = _blsr_u64(moves_bitboard)) {
        chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard);
        chess_t::square_t start_square = end_square + (to_move == WHITE ? 9 : -9);
        if (1ull << end_square & ~pin_lines[start_square]) {
            continue;    
        }
        if (to_move == WHITE ? end_square < 8 : end_square > 55) {
            for (uint32_t f = move_t::KNIGHT_PROMOTION_CAPTURE; f <= move_t::QUEEN_PROMOTION_CAPTURE; f++) {
                moves.add({start_square, end_square, (move_t::move_flags_t)f});
            }
        } else {
            moves.add({start_square, end_square, move_t::CAPTURE});
        }
    }
    uint64_t capture_right_move = (to_move == WHITE ? (pawns & ~file_h) >> 7 : (pawns & ~file_a) << 7);
    for (uint64_t moves_bitboard = capture_right_move & enemies & legal; moves_bitboard; moves_bitboard = _blsr_u64(moves_bitboard)) {
        chess_t::square_t end_square = (chess_t::square_t)_tzcnt_u64(moves_bitboard);
        chess_t::square_t start_square = end_square + (to_move == WHITE ? 7 : -7);
        if (1ull << end_square & ~pin_lines[start_square]) {
            continue;    
        }
        if (to_move == WHITE ? end_square < 8 : end_square > 55) {
            for (uint32_t f = move_t::KNIGHT_PROMOTION_CAPTURE; f <= move_t::QUEEN_PROMOTION_CAPTURE; f++) {
                moves.add({start_square, end_square, (move_t::move_flags_t)f});
            }
        } else {
            moves.add({start_square, end_square, move_t::CAPTURE});
        }
    }
    chess_t::square_t en_passant = board.game_state_stack.last()->en_passant;
    if (en_passant != null_square) {
        // check if en passant is pinned
        // skip if more than one allied pawn on rank
        uint64_t en_passant_bitboard = 1ull << en_passant;
        uint64_t capture_bitboard = 1ull << (en_passant + (to_move == WHITE ? 8 : -8));

        constexpr uint64_t en_passant_start_rank = to_move == WHITE ? rank_5 : rank_4;
        if (_mm_popcnt_u64(board.bitboards[to_move][PAWN] & en_passant_start_rank) == 1) {
            blockers &= ~capture_bitboard;
            blockers &= ~(board.bitboards[to_move][PAWN] & en_passant_start_rank);
            uint64_t rook_queen = board.bitboards[other_to_move][ROOK] | board.bitboards[other_to_move][QUEEN];
            uint64_t pinner = rook_queen & en_passant_start_rank;
            if (pinner) {
                chess_t::square_t pinner_square = (chess_t::square_t)_tzcnt_u64(pinner);
                if (gen_rook_moves(pinner_square, blockers, 0ull) & (board.bitboards[to_move][KING] & en_passant_start_rank)) {
                    return; // en passant is pinned, skip
                }
            }
        }


        // make capturing pawn end square legal if en passant square is legal
        // en passant is the only move where a piece can get captured without the capturing piece being on that square

        legal |= to_move == WHITE ? (legal & capture_bitboard) >> 8 : (legal & capture_bitboard) << 8;

        if (capture_left_move & en_passant_bitboard & legal ) {
            chess_t::square_t start_square = to_move == WHITE ? en_passant + 9 : en_passant - 9;
            if (en_passant_bitboard & pin_lines[start_square]) {
                moves.add({start_square, en_passant, move_t::EN_PASSANT_CAPTURE});
            }
        }
        if (capture_right_move & en_passant_bitboard & legal) {
            chess_t::square_t start_square = to_move == WHITE ? en_passant + 7 : en_passant - 7;
            if (en_passant_bitboard & pin_lines[start_square]) {
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

void chess_t::gen_castling_moves(square_t square, uint64_t blockers, uint64_t danger, move_array_t &moves) {
    color_t to_move = board.game_state_stack.last()->to_move;
    if (!(blockers & data::king_castling_clear[to_move][KINGSIDE]) &&
        board.game_state_stack.last()->castling_rights[to_move][KINGSIDE] &&
        !(danger & data::king_castling_safe[to_move][KINGSIDE])) {
        moves.add({square, data::king_castling_end_squares[to_move][KINGSIDE], move_t::KING_CASTLE}); 
    }
    if (!(blockers & data::king_castling_clear[to_move][QUEENSIDE]) &&
        board.game_state_stack.last()->castling_rights[to_move][QUEENSIDE] &&
        !(danger & data::king_castling_safe[to_move][QUEENSIDE])) {
        moves.add({square, data::king_castling_end_squares[to_move][QUEENSIDE], move_t::QUEEN_CASTLE}); 
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

uint64_t chess_t::gen_sliding_between(chess_t::square_t start_square, chess_t::square_t end_square) {
    return data::sliding_between_data[start_square][end_square];
}

uint64_t chess_t::gen_attackers(square_t square, uint64_t blockers) {
    color_t to_move = board.game_state_stack.last()->to_move;
    color_t other_to_move = (color_t)!to_move;
    uint64_t attackers = 0ull;
    // enemy's allies unused as only taking moves that intersect with enemy
    attackers |= gen_pawn_attacks(to_move, square) & board.bitboards[other_to_move][PAWN];
    attackers |= gen_knight_moves(square, 0ull) & board.bitboards[other_to_move][KNIGHT];
    
    uint64_t bishop_queen = board.bitboards[other_to_move][BISHOP] | board.bitboards[other_to_move][QUEEN];
    uint64_t rook_queen = board.bitboards[other_to_move][ROOK] | board.bitboards[other_to_move][QUEEN];

    attackers |= gen_bishop_moves(square, blockers, 0ull) & bishop_queen;
    attackers |= gen_rook_moves(square, blockers, 0ull) & rook_queen;
    attackers |= gen_king_moves(square, 0ull) & board.bitboards[other_to_move][KING];
    return attackers;
}

uint64_t chess_t::gen_sliding_danger(uint64_t blockers) {
    color_t to_move = board.game_state_stack.last()->to_move;
    color_t other_to_move = (color_t)!to_move;

    uint64_t danger = 0ull;

    for (uint64_t bishops = board.bitboards[other_to_move][BISHOP]; bishops; bishops = _blsr_u64(bishops)) {
        chess_t::square_t bishop_square = (chess_t::square_t)_tzcnt_u64(bishops);
        danger |= gen_bishop_moves(bishop_square, blockers, 0ull);
    }
    for (uint64_t rooks = board.bitboards[other_to_move][ROOK]; rooks; rooks = _blsr_u64(rooks)) {
        chess_t::square_t rook_square = (chess_t::square_t)_tzcnt_u64(rooks);
        danger |= gen_rook_moves(rook_square, blockers, 0ull);
    }
    for (uint64_t queens = board.bitboards[other_to_move][QUEEN]; queens; queens = _blsr_u64(queens)) {
        chess_t::square_t queen_square = (chess_t::square_t)_tzcnt_u64(queens);
        danger |= gen_queen_moves(queen_square, blockers, 0ull);
    }

    return danger;
}

uint64_t chess_t::gen_non_sliding_danger(uint64_t blockers) {
    color_t to_move = board.game_state_stack.last()->to_move;
    color_t other_to_move = (color_t)!to_move;

    uint64_t danger = 0ull;

    // assumes one king
    {
        chess_t::square_t king_square = (chess_t::square_t)_tzcnt_u64(board.bitboards[other_to_move][KING]);
        danger |= gen_king_moves(king_square, 0ull);
    }
    for (uint64_t pawns = board.bitboards[other_to_move][PAWN]; pawns; pawns = _blsr_u64(pawns)) {
        chess_t::square_t pawn_square = (chess_t::square_t)_tzcnt_u64(pawns);
        danger |= gen_pawn_attacks(other_to_move, pawn_square);
    }
    for (uint64_t knights = board.bitboards[other_to_move][KNIGHT]; knights; knights = _blsr_u64(knights)) {
        chess_t::square_t knight_square = (chess_t::square_t)_tzcnt_u64(knights);
        danger |= gen_knight_moves(knight_square, 0ull);
    }
    danger |= gen_sliding_danger(blockers);
    return danger;
}

void chess_t::gen_pins(uint64_t *pin_lines, square_t square, uint64_t danger, uint64_t sliding_danger, uint64_t blockers, uint64_t allies, uint64_t enemies) {    
    memset(pin_lines, 0, 64 * sizeof(uint64_t));
    
    color_t to_move = board.game_state_stack.last()->to_move;
    color_t other_to_move = (color_t)!to_move;

    uint64_t unpinned_allies = allies;

    for (uint64_t attacked_allies = allies & danger; attacked_allies; attacked_allies = _blsr_u64(attacked_allies)) {
        uint64_t attacked_ally = _blsi_u64(attacked_allies);
        if (sliding_danger & gen_queen_moves(square, blockers, 0ull) & attacked_ally) {
            unpinned_allies &= ~attacked_ally;
        }
    }

    uint64_t unpinned_blockers = unpinned_allies | enemies;

    uint64_t bishop_queen = board.bitboards[other_to_move][BISHOP] | board.bitboards[other_to_move][QUEEN];
    uint64_t rook_queen = board.bitboards[other_to_move][ROOK] | board.bitboards[other_to_move][QUEEN];

    uint64_t bishop_moves_from_square = gen_bishop_moves(square, unpinned_blockers, 0ull);
    uint64_t bishop_queen_attackers = bishop_moves_from_square & bishop_queen;

    for ( ; bishop_queen_attackers; bishop_queen_attackers = _blsr_u64(bishop_queen_attackers)) {
        chess_t::square_t bishop_square = (chess_t::square_t)_tzcnt_u64(bishop_queen_attackers);
        uint64_t pin = gen_bishop_moves(bishop_square, unpinned_blockers, 0ull) & bishop_moves_from_square;
        pin |= bishop_queen_attackers;

        uint64_t pinned = pin & allies;
        if (pinned) {
            chess_t::square_t pinned_square = (chess_t::square_t)_tzcnt_u64(pinned);
            pin_lines[pinned_square] |= pin;
        }
    }
    
    uint64_t rook_moves_from_square = gen_rook_moves(square, unpinned_blockers, 0ull);
    uint64_t rook_queen_attackers = rook_moves_from_square & rook_queen;
    
    for ( ; rook_queen_attackers; rook_queen_attackers = _blsr_u64(rook_queen_attackers)) {
        chess_t::square_t rook_square = (chess_t::square_t)_tzcnt_u64(rook_queen_attackers);
        uint64_t pin = gen_rook_moves(rook_square, unpinned_blockers, 0ull) & rook_moves_from_square;
        pin |= rook_queen_attackers;

        uint64_t pinned = pin & allies;
        if (pinned) {
            chess_t::square_t pinned_square = (chess_t::square_t)_tzcnt_u64(pinned);
            pin_lines[pinned_square] |= pin;
        }
    }

    for ( ; unpinned_allies; unpinned_allies = _blsr_u64(unpinned_allies)) {
        chess_t::square_t ally_square = (chess_t::square_t)_tzcnt_u64(unpinned_allies);
        pin_lines[ally_square] = 0xffffffffffffffffull;
    }
}

chess_t::move_array_t chess_t::gen_moves() {
    move_array_t moves;
    
    color_t to_move = board.game_state_stack.last()->to_move;

    uint64_t blockers = gen_blockers();
    uint64_t allies = gen_allies();
    uint64_t enemies = blockers & ~allies;

    chess_t::square_t king_square = (chess_t::square_t)_tzcnt_u64(board.bitboards[to_move][KING]);

    uint64_t checkers = gen_attackers(king_square, blockers);
    uint32_t num_checkers = (uint32_t)_mm_popcnt_u64(checkers);

     // xray through king (bitboard must ensure king can't move backward out of check)
    uint64_t danger_blockers = blockers & ~board.bitboards[to_move][KING];
    uint64_t sliding_danger = gen_sliding_danger(danger_blockers);
    uint64_t danger = sliding_danger | gen_non_sliding_danger(danger_blockers);
    
    // assumes one king
    {
        uint64_t moves_bitboard = gen_king_moves(king_square, allies);
        moves_bitboard &= ~danger;
        serialize_bitboard(king_square, moves_bitboard, enemies, moves);
    }

    // only king moves allowed when in double check
    if (num_checkers > 1) {
        return moves;
    }

    uint64_t pin_lines[64];
    gen_pins(pin_lines, king_square, danger, sliding_danger, blockers, allies, enemies);

    uint64_t legal = 0xffffffffffffffffull;
    if (num_checkers == 1) {
        legal = checkers;
        // inspired by https://peterellisjones.com/posts/generating-legal-chess-moves-efficiently/
        chess_t::square_t checking_square = (chess_t::square_t)_tzcnt_u64(checkers);
        switch (board.get_piece(checking_square).piece) {
        case BISHOP:
        case ROOK:
        case QUEEN:
            legal |= gen_sliding_between(king_square, checking_square);
            break;
        default:
            break;
        }
    } else {
        gen_castling_moves(king_square, blockers, danger, moves);
    }


    if (to_move == WHITE) {
        gen_pawn_moves<WHITE>(board.bitboards[to_move][PAWN], blockers, allies, enemies, legal, pin_lines, moves);
    } else {
        gen_pawn_moves<BLACK>(board.bitboards[to_move][PAWN], blockers, allies, enemies, legal, pin_lines, moves);    
    }
    for (uint64_t knights = board.bitboards[to_move][KNIGHT]; knights; knights = _blsr_u64(knights)) {
        chess_t::square_t knight_square = (chess_t::square_t)_tzcnt_u64(knights);
        uint64_t moves_bitboard = gen_knight_moves(knight_square, allies) & legal & pin_lines[knight_square];
        serialize_bitboard(knight_square, moves_bitboard, enemies, moves);
    }
    for (uint64_t bishops = board.bitboards[to_move][BISHOP]; bishops; bishops = _blsr_u64(bishops)) {
        chess_t::square_t bishop_square = (chess_t::square_t)_tzcnt_u64(bishops);
        uint64_t moves_bitboard = gen_bishop_moves(bishop_square, blockers, allies) & legal & pin_lines[bishop_square];
        serialize_bitboard(bishop_square, moves_bitboard, enemies, moves);
    }
    for (uint64_t rooks = board.bitboards[to_move][ROOK]; rooks; rooks = _blsr_u64(rooks)) {
        chess_t::square_t rook_square = (chess_t::square_t)_tzcnt_u64(rooks);
        uint64_t moves_bitboard = gen_rook_moves(rook_square, blockers, allies) & legal & pin_lines[rook_square];
        serialize_bitboard(rook_square, moves_bitboard, enemies, moves);
    }
    for (uint64_t queens = board.bitboards[to_move][QUEEN]; queens; queens = _blsr_u64(queens)) {
        chess_t::square_t queen_square = (chess_t::square_t)_tzcnt_u64(queens);
        uint64_t moves_bitboard = gen_queen_moves(queen_square, blockers, allies) & legal & pin_lines[queen_square];
        serialize_bitboard(queen_square, moves_bitboard, enemies, moves);
    }
    return moves;
}