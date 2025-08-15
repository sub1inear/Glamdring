#include "chess.h"
#include "data.h"

// precomp move generation inspired by https://www.talkchess.com/forum/viewtopic.php?topic_view=threads&p=175834&t=19699
static uint64_t gen_rook_mask(chess_t::square_t square) {
    uint64_t mask = 0;
    chess_t::square_t file = square % 8;
    chess_t::square_t rank = square / 8;
    for (chess_t::square_t f = file + 1; f < 7; f++) mask |= 1ull << (rank * 8 + f);
    for (chess_t::square_t f = file - 1; f > 0; f--) mask |= 1ull << (rank * 8 + f);
    for (chess_t::square_t r = rank + 1; r < 7; r++) mask |= 1ull << (r * 8 + file);
    for (chess_t::square_t r = rank - 1; r > 0; r--) mask |= 1ull << (r * 8 + file);
    return mask;
}
static uint64_t gen_rook_moves(chess_t::square_t square, uint64_t blockers) {
    uint64_t moves = 0;
    chess_t::square_t file = square % 8;
    chess_t::square_t rank = square / 8;
    for (chess_t::square_t f = file + 1; f <= 7; f++) {
        chess_t::square_t i = rank * 8 + f;
        moves |= 1ull << i;
        if (blockers & 1ull << i) {
            break;
        }
    }
    for (chess_t::square_t f = file - 1; f >= 0; f--) {
        chess_t::square_t i = rank * 8 + f;
        moves |= 1ull << i;
        if (blockers & 1ull << i) {
            break;
        }
    }
    for (chess_t::square_t r = rank + 1; r <= 7; r++) {
        chess_t::square_t i = r * 8 + file;
        moves |= 1ull << i;
        if (blockers & 1ull << i) {
            break;
        }
    }
    for (chess_t::square_t r = rank - 1; r >= 0; r--) {
        chess_t::square_t i = r * 8 + file;
        moves |= 1ull << i;
        if (blockers & 1ull << i) {
            break;
        }
    }
    return moves;
}
static uint64_t gen_bishop_mask(chess_t::square_t square) {
    uint64_t mask = 0;
    chess_t::square_t file = square % 8;
    chess_t::square_t rank = square / 8;
    for (chess_t::square_t f = file + 1, r = rank + 1; f < 7 && r < 7; f++, r++) mask |= 1ull << (r * 8 + f);
    for (chess_t::square_t f = file - 1, r = rank - 1; f > 0 && r > 0; f--, r--) mask |= 1ull << (r * 8 + f);
    for (chess_t::square_t f = file + 1, r = rank - 1; f < 7 && r > 0; f++, r--) mask |= 1ull << (r * 8 + f);
    for (chess_t::square_t f = file - 1, r = rank + 1; f > 0 && r < 7; f--, r++) mask |= 1ull << (r * 8 + f);
    return mask;
}
static uint64_t gen_bishop_moves(chess_t::square_t square, uint64_t blockers) {
    uint64_t moves = 0;
    chess_t::square_t file = square % 8;
    chess_t::square_t rank = square / 8;
    for (chess_t::square_t f = file + 1, r = rank + 1; f <= 7 && r <= 7; f++, r++) {
        chess_t::square_t i = r * 8 + f;
        moves |= 1ull << i;
        if (blockers & 1ull << i) {
            break;
        }
    }
    for (chess_t::square_t f = file - 1, r = rank - 1; f >= 0 && r >= 0; f--, r--) {
        chess_t::square_t i = r * 8 + f;
        moves |= 1ull << i;
        if (blockers & 1ull << i) {
            break;
        }
    }
    for (chess_t::square_t f = file + 1, r = rank - 1; f <= 7 && r >= 0; f++, r--) {
        chess_t::square_t i = r * 8 + f;
        moves |= 1ull << i;
        if (blockers & 1ull << i) {
            break;
        }
    }
    for (chess_t::square_t f = file - 1, r = rank + 1; f >= 0 && r <= 7; f--, r++) {
        chess_t::square_t i = r * 8 + f;
        moves |= 1ull << i;
        if (blockers & 1ull << i) {
            break;
        }
    }
    return moves;
}
static void print_pext(FILE *fout, bool rook) {
    static uint64_t offset = 0;
    for (chess_t::square_t square = 0; square < 64; square++) {
        uint64_t mask = rook ? gen_rook_mask(square) : gen_bishop_mask(square);
        uint64_t idx = offset;
        fprintf(fout, "    { %lluull, &pext_move_data[%llu] },\n", mask, idx);
        uint64_t size = 1ull << intrin::popcnt(mask); // count 1's in mask
        offset += size;
    }
}
static void print_pext_move_data(FILE *fout, bool rook) {
    uint32_t total = 0;
    for (chess_t::square_t square = 0; square < 64; square++) {
        uint64_t mask = rook ? gen_rook_mask(square) : gen_bishop_mask(square);
        uint64_t size = 1ull << intrin::popcnt(mask);
        for (uint32_t i = 0; i < size; i++, total++) {
            if (total % 8 == 0) {
                fputs("    ", fout);
            }
            uint64_t blockers = intrin::pdep(i, mask); // deposit bits according to mask
            uint64_t moves = rook ? gen_rook_moves(square, blockers) : gen_bishop_moves(square, blockers);
            fprintf(fout, "%lluull, ", moves);
            if (total % 8 == 7) {
                fputc('\n', fout);
            }
        }        
    }
}
static uint64_t gen_knight_moves_slow(chess_t::square_t square) {
    uint64_t moves = 0;
    chess_t::square_t file = square % 8;
    chess_t::square_t rank = square / 8;
    if (rank > 1) {
        if (file < 7) {
            moves |= 1ull << (square - 15);
        }
        if (file > 0) {
            moves |= 1ull << (square - 17);
        }
    }
    if (file > 1) {
        if (rank > 0) {
            moves |= 1ull << (square - 10);
        }
        if (rank < 7) {
            moves |= 1ull << (square + 6);
        }
    }
    if (rank < 6) {
        if (file < 7) {
            moves |= 1ull << (square + 17);
        }
        if (file > 0) {
            moves |= 1ull << (square + 15);
        }
    }
    if (file < 6) {
        if (rank > 0) {
            moves |= 1ull << (square - 6);
        }
        if (rank < 7) {
            moves |= 1ull << (square + 10);
        }
    }
    return moves;
}
static uint64_t gen_king_moves_slow(chess_t::square_t square) {
    uint64_t moves = 0;
    chess_t::square_t file = square % 8;
    chess_t::square_t rank = square / 8;
    if (rank > 0) {
        moves |= 1ull << (square - 8);
        if (file > 0) {
            moves |= 1ull << (square - 9);
        }
        if (file < 7) {
            moves |= 1ull << (square - 7); 
        }
    }
    if (rank < 7) {
        moves |= 1ull << (square + 8);
        if (file > 0) {
            moves |= 1ull << (square + 7);
        }
        if (file < 7) {
            moves |= 1ull << (square + 9); 
        }
    }
    if (file > 0) {
        moves |= 1ull << (square - 1);
    }
    if (file < 7) {
        moves |= 1ull << (square + 1);
    }
    return moves;
}
static void print_non_magic_data(FILE *fout, uint64_t (*func)(chess_t::square_t square)) {
    for (uint32_t i = 0; i < 16; i++) {
        fputs("    ", fout);
        for (uint32_t j = 0; j < 4; j++) {
            chess_t::square_t square = i * 4 + j;
            fprintf(fout, "%lluull, ", func(square));
        }
        fputc('\n', fout);                      
    }
}
static uint64_t gen_pawn_attacks(chess_t::color_t color, chess_t::square_t square) {
    uint64_t pawn = 1ull << square;
    constexpr uint64_t file_a = 0x101010101010101;
    constexpr uint64_t file_h = 0x8080808080808080;
    if (color == chess_t::WHITE) {
        return (pawn & ~file_a) >> 9 | (pawn & ~file_h) >> 7;
    }
    return (pawn & ~file_h) << 9 | (pawn & ~file_a) << 7;
}
static void print_pawn_attack_data(FILE *fout) {
    for (uint32_t color = 0; color < 2; color++) {
        fputs("    {", fout);
        for (uint32_t i = 0; i < 16; i++) {
            fputs("\n        ", fout);
            for (uint32_t j = 0; j < 4; j++) {
                chess_t::square_t square = i * 4 + j;
                fprintf(fout, "%lluull, ", gen_pawn_attacks((chess_t::color_t)color, square));
            }
        }
        fputs("\n    },\n", fout);
    }
}
static uint64_t gen_sliding_between(chess_t::square_t start_square, chess_t::square_t end_square) {
    uint64_t start_bitboard = 1ull << start_square;
    uint64_t end_bitboard = 1ull << end_square;
    uint64_t blockers = start_bitboard | end_bitboard;

    uint64_t rook_moves = gen_rook_moves(start_square, blockers);
    if (rook_moves & end_bitboard) {
        return rook_moves & gen_rook_moves(end_square, blockers);
    } else {
        uint64_t bishop_moves = gen_bishop_moves(start_square, blockers);
        if (bishop_moves & end_bitboard) {
            return bishop_moves & gen_bishop_moves(end_square, blockers);
        } else {
            return 0ull;
        }
    }            
}
static void print_sliding_between_data(FILE *fout) {
    for (chess_t::square_t start_square = 0; start_square < 64; start_square++) {
        fputs("    {", fout);
        for (uint32_t i = 0; i < 16; i++) {
            fputs("\n        ", fout);
            for (uint32_t j = 0; j < 4; j++) {
                chess_t::square_t end_square = i * 4 + j;
                fprintf(fout, "%lluull, ", gen_sliding_between(start_square, end_square));
            }
        }
        fputs("\n    },\n", fout);
    }
}

void chess_t::gen_precomp_data() {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    FILE *fout = fopen("data.cpp", "w");
    if (fout == nullptr) {
        printf("fopen() in gen_precomp_data() failed: %s", strerror(errno));
        exit(1);
    }
    fputs("#include \"data.h\"\n\n" 
          "namespace data {\n"
          "const pext_t bishop_pext[] = {\n",
          fout
    );
    print_pext(fout, false);
    fputs("};"
          "\nconst pext_t rook_pext[] = {\n",
          fout
    );
    print_pext(fout, true);
    fputs("};\n"
          "const uint64_t pext_move_data[] = {\n",
          fout
    );
    print_pext_move_data(fout, false);
    print_pext_move_data(fout, true);
    fputs("};\n"
          "const uint64_t knight_move_data[] = {\n",
          fout
    );
    print_non_magic_data(fout, gen_knight_moves_slow);
    fputs("};\n"
          "const uint64_t king_move_data[] = {\n",
          fout
    );
    print_non_magic_data(fout, gen_king_moves_slow);
    fputs("};\n"
          "const uint64_t pawn_attack_data[][64] = {\n",
          fout
    );
    print_pawn_attack_data(fout);
    fputs("};\n"
          "const uint64_t sliding_between_data[64][64] = {\n",
          fout
    );
    print_sliding_between_data(fout);
    fputs("};\n"
          "}",
          fout
    );
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    printf("Took %lli ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}
