#include "chess.h"

chess_t::transposition_table_t::transposition_entry_t chess_t::transposition_table_t::lookup(uint64_t key) {
    uint32_t idx = key % entries;
    return table[idx];
}
void chess_t::transposition_table_t::store(int32_t eval, uint8_t move_idx, uint64_t key, int32_t alpha, int32_t beta, uint8_t depth) {
    uint32_t idx = key % entries;

    if (table[idx].data.depth >= depth) {
        return; // only store if depth is greater, unwritten entries have a depth of 0
    }

    transposition_data_t data = { eval, move_idx, depth, EXACT };
    if (eval <= alpha) {
        data.type = UPPERBOUND;
    } else if (eval >= beta) {
        data.type = LOWERBOUND;
    }
    table[idx].data = data;
    table[idx].data_xor_key = (uint64_t)data ^ key;
}