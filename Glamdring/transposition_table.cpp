#include "chess.h"

chess_t::transposition_table_t::transposition_result_t chess_t::transposition_table_t::lookup(uint64_t key, int32_t alpha, int32_t beta, uint8_t depth) {
    uint32_t idx = key % size;
    transposition_data_t data = table[idx].data;
    
    // check if data xored with transposition table's key is the same as data xored with our key
    if (table[idx].data_xor_key == ((uint64_t)data ^ key)) {
        if (data.depth >= depth) {
            switch (data.type) {
            case EXACT:
                return data.result;
            case UPPERBOUND:
                if (data.result.eval <= alpha) {
                    return data.result;
                }
                break;
            case LOWERBOUND:
                if (data.result.eval >= beta) {
                    return data.result;
                }
                break;
            }
        }
    }
    return { { -1, -1, move_t::QUIET }, 0 };
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