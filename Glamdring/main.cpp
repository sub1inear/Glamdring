#include "chess.h"
#include "data.h"

int main(int argc, char **argv) {
    chess_t chess;

    bool result = chess.opening_book.set_book("../../OpeningBooks/Titans.bin");
    if (!result) {
        chess.print_uci("fopen() in opening_book_t::set_book() failed: %s", "hello");
    }

    chess.uci();
}