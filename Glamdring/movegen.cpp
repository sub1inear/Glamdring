#include "chess.h"
#include "data.h"

void chess_t::serialize_bitboard(square_t start_square, uint64_t moves_bitboard, uint64_t enemies, move_array_t &moves) {
    for ( ; moves_bitboard; moves_bitboard &= moves_bitboard - 1 /* remove lsb */) {
        chess_t::square_t end_square = _tzcnt_u64(moves_bitboard); // count trailing zeros
        uint64_t lsb = _blsi_u64(moves_bitboard); // extract lsb
        move_t::move_flags_t flags = lsb & enemies ? move_t::CAPTURE : move_t::QUIET;
        moves.add({start_square, end_square, flags});
    }
}

void chess_t::gen_knight_moves(square_t square, uint64_t allies, uint64_t enemies, move_array_t &moves) {
    uint64_t moves_bitboard = knight_move_data[square];
    moves_bitboard &= ~allies;
    serialize_bitboard(square, moves_bitboard, enemies, moves);
}
void chess_t::gen_bishop_moves(square_t square, uint64_t blockers, uint64_t allies, uint64_t enemies, move_array_t &moves) {
    magic_t magic = rook_magics[square];
    uint64_t key = (blockers & magic.mask) * magic.magic >> magic.shift;
    uint64_t moves_bitboard = magic_move_data[magic.idx + key];
    moves_bitboard &= ~allies;
    serialize_bitboard(square, moves_bitboard, enemies, moves);
}

void chess_t::gen_rook_moves(square_t square, uint64_t blockers, uint64_t allies, uint64_t enemies, move_array_t &moves) {
    magic_t magic = rook_magics[square];
    uint64_t key = (blockers & magic.mask) * magic.magic >> magic.shift;
    uint64_t moves_bitboard = magic_move_data[magic.idx + key];
    moves_bitboard &= ~allies;
    serialize_bitboard(square, moves_bitboard, enemies, moves);
}

void chess_t::gen_queen_moves(square_t square, uint64_t blockers, uint64_t allies, uint64_t enemies, move_array_t &moves) {
    magic_t rook_magic = rook_magics[square];
    uint64_t rook_key = (blockers & rook_magic.mask) * rook_magic.magic >> rook_magic.shift;
    uint64_t rook_moves_bitboard = magic_move_data[rook_magic.idx + rook_key];

    magic_t bishop_magic = rook_magics[square];
    uint64_t bishop_key = (blockers & bishop_magic.mask) * bishop_magic.magic >> bishop_magic.shift;
    uint64_t bishop_moves_bitboard = magic_move_data[bishop_magic.idx + bishop_key];
        
    uint64_t moves_bitboard = rook_moves_bitboard | bishop_moves_bitboard;
    moves_bitboard &= ~allies;
    serialize_bitboard(square, moves_bitboard, enemies, moves);
}

void chess_t::gen_king_moves(square_t square, uint64_t allies, uint64_t enemies, move_array_t &moves) {
    uint64_t moves_bitboard = king_move_data[square];
    moves_bitboard &= ~allies;
    serialize_bitboard(square, moves_bitboard, enemies, moves);
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
    for (uint64_t pawns = board.bitboards[to_move][PAWN]; pawns; pawns &= pawns - 1 /* remove lsb */) {
        chess_t::square_t square = _tzcnt_u64(pawns); // count trailing zeros
    }
    for (uint64_t knights = board.bitboards[to_move][KNIGHT]; knights; knights &= knights - 1 /* remove lsb */) {
        chess_t::square_t square = _tzcnt_u64(knights); // count trailing zeros
        gen_knight_moves(square, allies, enemies, moves);
    }
    for (uint64_t bishops = board.bitboards[to_move][BISHOP]; bishops; bishops &= bishops - 1 /* remove lsb */) {
        chess_t::square_t square = _tzcnt_u64(bishops); // count trailing zeros
        gen_bishop_moves(square, blockers, allies, enemies, moves);
    }
    for (uint64_t rooks = board.bitboards[to_move][ROOK]; rooks; rooks &= rooks - 1 /* remove lsb */) {
        chess_t::square_t square = _tzcnt_u64(rooks); // count trailing zeros
        gen_rook_moves(square, blockers, allies, enemies, moves);
    }
    for (uint64_t queens = board.bitboards[to_move][QUEEN]; queens; queens &= queens - 1 /* remove lsb */) {
        chess_t::square_t square = _tzcnt_u64(queens); // count trailing zeros
        gen_queen_moves(square, blockers, allies, enemies, moves);
    }
    // assumes one king
    {
        chess_t::square_t square = _tzcnt_u64(board.bitboards[to_move][KING]);
        gen_king_moves(square, allies, enemies, moves);
    }
    return moves;
}