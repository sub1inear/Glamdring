#include "chess.h"
#include "data.h"

int main(int argc, char **argv) {
    chess_t chess;

    bool result = chess.opening_book.set_book("Titans.bin");
    if (!result) {
        chess.print_uci("fopen() in opening_book_t::set_book() failed: %s\n", strerror(errno));
        return 1;
    }

    chess.uci();
}