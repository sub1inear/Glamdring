#include "chess.h"

static uint64_t random64() {
    static uint64_t seed = 1234609812624019826;
    seed = seed * 6364136223846793005 + 1442695040888963407;
    return seed;
}
static uint64_t random64_low_bits() {
    return random64() & random64();
}
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
static chess_t::magic_t gen_magic(chess_t::square_t square, bool rook) {
    uint64_t mask = rook ? gen_rook_mask(square) : gen_bishop_mask(square);
    
    uint64_t blockers_bits = __popcnt64(mask);
    uint64_t blockers_size = 1ull << blockers_bits;
    
    uint64_t best_magic = 0;
    int best_shift = 0;

    uint64_t precomp_moves[1 << 12];
    uint64_t moves[1 << 15];
    uint64_t used[1 << 15] {0};

    uint64_t generation = 0;

    for (uint32_t i = 0; i < 1 << 12; i++) {
       uint64_t blockers = _pdep_u64(i, mask);
       precomp_moves[i] = rook ? gen_rook_moves(square, blockers) : gen_bishop_moves(square, blockers);
    }
    for (uint32_t shift = 64 - 12; shift < 64 - 7; shift++) {
        for (uint32_t i = 0; i < 1'000'000'000ul; i++) {
            generation++;

            bool succeeded = true;
            uint64_t magic = random64_low_bits();
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
    return { 0, best_magic, mask, best_shift };
}
void chess_t::gen_magics() {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::cout << "constexpr chess_t::magic_t rook_magics = {\n";
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            magic_t magic;
            do {
                magic = gen_magic(i * 4 + j, true);
            } while (magic.magic == 0);
            std::cout << "    ";
            magic.print();
            std::cout << "\n";
        }
    }
    std::cout << "};\nconstexpr chess_t::magic_t bishop_magics = {\n";
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            magic_t magic;
            do {
                magic = gen_magic(i * 4 + j, true);
            } while (magic.magic == 0);
            std::cout << "    ";
            magic.print();
            std::cout << "\n";
        }
    }
    std::cout << "};\n";
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}
