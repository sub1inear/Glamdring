#include "chess.h"
#include "data.h"

// heavily inspired by https://www.talkchess.com/forum/viewtopic.php?topic_view=threads&p=175834&t=19699
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
static magic_t gen_magic(chess_t::square_t square, bool rook) {
    uint64_t mask = rook ? gen_rook_mask(square) : gen_bishop_mask(square);
    uint64_t blockers_size = 1ull <<  __popcnt64(mask); // 2 ^ population count of mask
    
    uint64_t best_magic = 0;
    uint32_t best_shift = 0;

    uint64_t precomp_moves[1 << 12];
    uint64_t moves[1 << 12];
    uint64_t used[1 << 12] {0};

    uint64_t generation = 0;

    uint64_t seed = 1234609812624019826;

    for (uint32_t i = 0; i < 1 << 12; i++) {
       uint64_t blockers = _pdep_u64(i, mask);
       precomp_moves[i] = rook ? gen_rook_moves(square, blockers) : gen_bishop_moves(square, blockers);
    }
    for (uint32_t shift = 64 - 12; shift < 64 - 6; shift++) {
        for (uint32_t i = 0; i < 1'000'000'000ul; i++) {
            generation++;

            bool succeeded = true;
            seed = seed * 6364136223846793005 + 1442695040888963407;
            uint64_t magic = seed;
            seed = seed * 6364136223846793005 + 1442695040888963407;
            magic &= seed;

            // if magic does not spread bits in key well enough, skip
            if (__popcnt64((mask * magic) & 0xff00000000000000) < 6) {
                continue;
            }
            for (uint64_t j = 0; j < blockers_size; j++) {
                uint64_t blockers = _pdep_u64(j, mask);
                uint64_t move = precomp_moves[j];
                uint64_t key = blockers * magic >> shift;
                
                if (used[key] != generation) {
                    moves[key] = move;
                    used[key] = generation;
                } else if (moves[key] != move) {
                    succeeded = false;
                    break;
                }
            }
            if (succeeded) {
                best_magic = magic;
                best_shift = shift;
                break;
            }
        }
    }
    return { best_magic, mask, 0, best_shift };
}
void gen_piece_magics(magic_t *magics, bool rook) {
    #pragma omp parallel for num_threads(20) schedule(dynamic)
    for (chess_t::square_t square = 0; square < 64; square++) {
        do {
             magics[square] = gen_magic(square, rook);
        } while (magics[square].magic == 0);
    }
}
static void print_magics(std::ofstream &fout, magic_t *magics, uint32_t &offset) {
    for (chess_t::square_t square = 0; square < 64; square++) {
        magics[square].idx = offset;
        offset += 1u << (64 - magics[square].shift);
        fout << "    { " << magics[square].magic << "ull, " << magics[square].mask << "ull, " << magics[square].idx << "u, " << magics[square].shift << "u },\n";
    }
}
static void print_magic_move_data(std::ofstream &fout, magic_t *magics, bool rook) {
    uint32_t total = 0;
    for (chess_t::square_t square = 0; square < 64; square++) {
        uint64_t mask = rook ? gen_rook_mask(square) : gen_bishop_mask(square);
        uint64_t blockers_size = 1ull << __popcnt64(mask);
        uint32_t moves_size = 1u << (64 - magics[square].shift);
        uint64_t moves[1 << 12]; // allocate on heap? 
        for (uint32_t i = 0; i < blockers_size; i++) {
            uint64_t blockers = _pdep_u64(i, mask);
            uint64_t key = blockers * magics[square].magic >> magics[square].shift;
            uint64_t move = rook ? gen_rook_moves(square, blockers) : gen_bishop_moves(square, blockers);
            moves[key] = move;
        }
        for (uint32_t i = 0; i < moves_size; i++, total++) {
            if (total % 8 == 0) {
                fout << "    ";
            }
            fout << moves[i] << "ull, ";
            if (total % 8 == 7) {
                fout << "\n";
            }
        }
    }
}
void chess_t::gen_magics() {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    magic_t magics[2][64];
    uint32_t offset = 0;
    std::ofstream fout("data.cpp");
    gen_piece_magics((magic_t *)&magics[0], false);
    gen_piece_magics((magic_t *)&magics[1], true);
    fout << "#include \"data.h\"\n\nconst magic_t bishop_magics[] = {\n";
    print_magics(fout, (magic_t *)&magics[0], offset);
    fout << "};\nconst magic_t rook_magics[] = {\n";
    print_magics(fout, (magic_t *)&magics[1], offset);
    fout << "};\nconst uint64_t magic_move_data[] = {\n";
    print_magic_move_data(fout, (magic_t *)&magics[0], false);
    print_magic_move_data(fout, (magic_t *)&magics[1], true);
    fout << "};";
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}
