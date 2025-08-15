#include "chess.h"
#include "data.h"

template <chess_t::color_t color>
int32_t chess_t::count_material() {
    int32_t material = 0;
    constexpr uint32_t flip = color == WHITE ? 0 : 56; // https://www.chessprogramming.org/Color_Flipping#Flipping_an_8x8_Board
    for (uint64_t pawns = board.bitboards[color][PAWN]; pawns; pawns = intrin::blsr(pawns)) {
        chess_t::square_t pawn_square = (chess_t::square_t)intrin::ctz(pawns) ^ flip;
        material += data::piece_square_values[PAWN][pawn_square];
    }
    for (uint64_t knights = board.bitboards[color][KNIGHT]; knights; knights = intrin::blsr(knights)) {
        chess_t::square_t knight_square = (chess_t::square_t)intrin::ctz(knights) ^ flip;
        material += data::piece_square_values[KNIGHT][knight_square];
    }
    for (uint64_t bishops = board.bitboards[color][BISHOP]; bishops; bishops = intrin::blsr(bishops)) {
        chess_t::square_t bishop_square = (chess_t::square_t)intrin::ctz(bishops) ^ flip;
        material += data::piece_square_values[BISHOP][bishop_square];
    }
    for (uint64_t rooks = board.bitboards[color][ROOK]; rooks; rooks = intrin::blsr(rooks)) {
        chess_t::square_t rook_square = (chess_t::square_t)intrin::ctz(rooks) ^ flip;
        material += data::piece_square_values[ROOK][rook_square];
    }
    for (uint64_t queens = board.bitboards[color][QUEEN]; queens; queens = intrin::blsr(queens)) {
        chess_t::square_t queen_square = (chess_t::square_t)intrin::ctz(queens) ^ flip;
        material += data::piece_square_values[QUEEN][queen_square];
    }
    return material;
}

int32_t chess_t::eval() {
    int32_t eval = 0;
    eval += count_material<WHITE>();
    eval -= count_material<BLACK>();
    int32_t color_coef = board.game_state_stack.last()->to_move == WHITE ? 1 : -1;
    return eval * color_coef;
}