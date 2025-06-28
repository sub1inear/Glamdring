#include "chess.h"
#include "data.h"

int32_t chess_t::eval() {
    int32_t eval = 0;
    for (uint32_t piece = 0; piece < 5; piece++) {
        
        uint32_t pieces_diff = (uint32_t)(_mm_popcnt_u64(board.bitboards[WHITE][piece]) - _mm_popcnt_u64(board.bitboards[BLACK][piece]));    
        eval += data::piece_values[piece] * pieces_diff;
    }
    int32_t color_coef = board.game_state_stack.last()->to_move == WHITE ? 1 : -1;
    return eval * color_coef;
}