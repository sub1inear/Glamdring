#include "chess.h"

chess_t::transposition_table_t::transposition_entry_t chess_t::transposition_table_t::lookup(uint64_t key) {
    uint32_t idx = key % size;
    return table[idx];
}
void chess_t::transposition_table_t::store(transposition_result_t result, uint64_t key, int32_t alpha, int32_t beta, uint8_t depth) {
    transposition_data_t data = { result, depth, EXACT };
    if (result.eval <= alpha) {
        data.type = UPPERBOUND;
    } else if (result.eval >= beta) {
        data.type = LOWERBOUND;
    }
    uint32_t idx = key % size;
    table[idx].data = data;
    table[idx].data_xor_key = (uint64_t)data ^ key;
}