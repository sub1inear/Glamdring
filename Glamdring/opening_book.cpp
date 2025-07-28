#include "chess.h"

void chess_t::opening_book_t::polyglot_entry_t::byteswap_key() {
    zobrist_key = _byteswap_uint64(zobrist_key);
}

void chess_t::opening_book_t::polyglot_entry_t::byteswap_non_key() {
    move = _byteswap_ushort(move);
    weight = _byteswap_ushort(weight);
    learn = _byteswap_ushort(learn);
}

bool chess_t::opening_book_t::set_book(const char *filename) {
    book = fopen(filename, "r");
    if (book == nullptr) {
        return false;
    }
    fseek(book, -1, SEEK_END);
    last = ftell(book) / sizeof(polyglot_entry_t);
    return true;
}

bool chess_t::opening_book_t::lookup(board_t &board, move_t &best_move) {

    uint64_t zobrist_key = board.get_polyglot_key();

    int32_t left = 0;
    int32_t right = last;

    while (left <= right) {
        int32_t middle = left + (right - left) / 2;
        int32_t idx = middle * sizeof(polyglot_entry_t);
        fseek(book, idx, SEEK_SET);
         
        polyglot_entry_t entry;
        fread(&entry, sizeof(entry), 1, book);

        // PolyGlot is stored big-endian
        entry.byteswap_key();

        if (entry.zobrist_key < zobrist_key) {
            left = middle + 1;
        } else if (entry.zobrist_key > zobrist_key) {
            right = middle - 1;
        } else {
            entry.byteswap_non_key();

            array_t<polyglot_entry_t, 100> entries;
            entries.add(entry);

            while (true) {
                fread(entries.end(), sizeof(*entries.end()), 1, book);
                entries.end()->byteswap_key();
                if (entries.end()->zobrist_key != zobrist_key) {
                    break;
                }
                entries.end()->byteswap_non_key();
                entries.size++;
            }

            fseek(book, idx - sizeof(entry), SEEK_SET);
            while (true) {
                fread(entries.end(), sizeof(*entries.end()), 1, book);
                fseek(book, -(int32_t)(sizeof(entry) * 2), SEEK_CUR);
                entries.end()->byteswap_key();
                if (entries.end()->zobrist_key != zobrist_key) {
                    break;
                }
                entries.end()->byteswap_non_key();
                entries.size++;
            }
            uint32_t sum = 0;
            for (uint32_t i = 0; i < entries.size; i++) {
                sum += entries[i].weight;
            }

            if (sum == 0) {
                break;
            }

            uint32_t r = rand() % sum;
            
            uint32_t min = 0;
            for (uint32_t i = 0; i < entries.size; i++) {
                if (min + entries[i].weight > r) {
                    best_move = { board, entries[i].move };
                    return true;        
                }
                min += entries[i].weight;
            }
            break;
            
        }
    }
    return false;
}