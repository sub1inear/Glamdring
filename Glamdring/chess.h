#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <immintrin.h>

template <typename T, uint32_t S>
class array_t {
public:
    T data[S];
    uint32_t size;

    array_t(uint32_t size = 0) : size(size) {}
    void add(T item) {
        data[size++] = item;
    }
    T *last() {
        return &data[size - 1];
    }
    T *next() {
        size++;
        return last();
    }
    T *pop() {
        size--;
        return last(); 
    }
    T &operator[](uint32_t idx) {
        return data[idx];
    }
};

class chess_t {
public:
    typedef int32_t square_t;
    typedef int8_t packed_square_t;

    enum color_t : uint8_t {
        WHITE,
        BLACK
    };
    enum piece_t : uint8_t {
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING,
        CLEAR
    };
    enum castling_side_t : uint8_t {
        KINGSIDE,
        QUEENSIDE
    };
    enum file_rank_t : square_t {
        A8, B8, C8, D8, E8, F8, G8, H8,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A1, B1, C1, D1, E1, F1, G1, H1,
    };

    static constexpr square_t null_square = -1;
    static constexpr uint32_t max_ply = 1000;
    static constexpr uint32_t max_moves = 218;

    chess_t() {}

    // TODO: use 1 byte
    class piece_color_t {
    public:
        piece_t piece;
        color_t color;
        piece_color_t() {}
        piece_color_t(piece_t piece, color_t color) : piece(piece), color(color) {};
        piece_color_t(char c);
        operator char();
    };

    struct piece_square_t {
        piece_color_t piece;
        square_t square;
    };

    class board_t;
    class move_t;

    // utils.cpp
    static square_t file_rank_to_square(square_t file, square_t rank);
    static void square_to_file_rank(square_t square, char *out);
    static void print_square(square_t square, FILE *out = stdout);
    static void print_bitboard(uint64_t bitboard);
    static char piece_to_char(piece_t piece);
    static piece_t char_to_piece(char c);

    // board.cpp
    class board_t {
    public:
        alignas(64)
        piece_color_t board[64];
        uint64_t bitboards[2][6];

        struct game_state_t {
            uint32_t half_move_clock;
            uint32_t full_moves;
            square_t en_passant;
            bool castling_rights[2][2];
            color_t to_move;
            piece_color_t captured_piece;
        };
        array_t<game_state_t, max_ply> game_state_stack;

        board_t() {}
        piece_color_t get_piece(square_t square) {
            return board[square];
        }
        void set_piece(square_t square, piece_color_t piece) {
            board[square] = piece;
            bitboards[piece.color][piece.piece] |= 1ull << square;
        }
        void clear_piece_bitboard(square_t square, piece_color_t piece) {
            bitboards[piece.color][piece.piece] &= ~(1ull << square);
        }
        void clear_piece(square_t square, piece_color_t piece) {
            board[square].piece = CLEAR;
            clear_piece_bitboard(square, piece);
        }
        
        void print(FILE *out = stdout);
        void clear();
        void load_fen(const char *fen);
        void make_move(move_t move);
        void undo_move(move_t move);
    };
    board_t board;

    class move_t {
    public:
        /*
        From https://www.chessprogramming.org/Encoding_Moves#From-To_Based
        Binary Format:
        0b x x x x
           ^ ^ ^ ^
           | | | | -- Special 1
           | | | ---- Special 2
           | | ------ Capture
           | -------- Promotion
        */
        enum move_flags_t : uint8_t {
            QUIET,
            DOUBLE_PAWN_PUSH,
            KING_CASTLE,
            QUEEN_CASTLE,
            CAPTURE,
            EN_PASSANT_CAPTURE,
            PROMOTION = 8,
            KNIGHT_PROMOTION = 8,
            BISHOP_PROMOTION,
            ROOK_PROMOTION,
            QUEEN_PROMOTION,
            KNIGHT_PROMOTION_CAPTURE,
            BISHOP_PROMOTION_CAPTURE,
            ROOK_PROMOTION_CAPTURE,
            QUEEN_PROMOTION_CAPTURE,
        };
        packed_square_t from/*: 6 bits*/;
        packed_square_t to/*: 6 bits*/;
        move_flags_t flags/*: 4 bits*/;
        move_t() {}
        move_t(square_t from, square_t to, move_flags_t flags) : from(from), to(to), flags(flags) {}
        move_t(board_t &board, const char *str); // utils.cpp
        move_t(uint16_t value) {
            // TODO: use _bext_u32
            from = value >> 10;
            to = (value >> 4) & 0x3f;
            flags = (move_flags_t)(value & 0xf);
        }
        uint16_t pack() {
            return from << 10 | to << 4 | flags;
        }
        bool is_capture() {
            return flags & CAPTURE;
        }
        bool is_promotion() {
            return flags & PROMOTION;
        }
        bool is_castling() {
            return flags == KING_CASTLE || flags == QUEEN_CASTLE;
        }
        castling_side_t get_castling() {
            return (castling_side_t)(flags & 0x1);
        }
        piece_t get_promotion() {
            return (piece_t)((flags & 0x3) + 1); // TODO: remove + 1 by starting piece_t with knight?
        }
        void print(FILE *out = stdout) {
            print_square(from, out);
            print_square(to, out);
            if (is_promotion()) {
                putc(piece_to_char(get_promotion()), out);
            }
        }
    };
    typedef array_t<move_t, max_moves> move_array_t; 

    // movegen.cpp
    static void serialize_bitboard(square_t square, uint64_t moves_bitboard, uint64_t enemies, move_array_t &moves);
    template <color_t to_move>
    void gen_pawn_moves(uint64_t pawns, uint64_t blockers, uint64_t allies, uint64_t enemies, uint64_t legal, uint64_t *pin_lines, move_array_t &moves);
    uint64_t gen_pawn_attacks(color_t to_move, square_t square);
    uint64_t gen_knight_moves(square_t square, uint64_t allies);
    uint64_t gen_bishop_moves(square_t square, uint64_t blockers, uint64_t allies);
    uint64_t gen_rook_moves(square_t square, uint64_t blockers, uint64_t allies);
    uint64_t gen_queen_moves(square_t square, uint64_t blockers, uint64_t allies);
    uint64_t gen_king_moves(square_t square, uint64_t allies);
    void gen_castling_moves(square_t square, uint64_t blockers, uint64_t danger, move_array_t &moves);

    uint64_t gen_sliding_between(square_t start_square, square_t end_square);
    uint64_t gen_blockers();
    uint64_t gen_allies();
    
    // generates the attackers for a square
    uint64_t gen_attackers(square_t square, uint64_t blockers);
    // "king danger" terminology from https://peterellisjones.com/posts/generating-legal-chess-moves-efficiently/
    // generates all opponent's attacked squares (with the king removed to ensure it cannot go back out of check)
    uint64_t gen_king_danger_squares(uint64_t blockers);
    uint64_t gen_pinning_danger(square_t square);
    // generates all pin lines for a square
    void gen_pins(uint64_t *pin_lines, square_t square, uint64_t allies, uint64_t enemies); // TODO: use reference?
    move_array_t gen_moves();
    
    // precomp.cpp
    static void gen_precomp_data();

    // test.cpp
    uint64_t perft(uint32_t depth, bool root = true);
    uint32_t test();

    // search.cpp
    move_t search();

    // uci.cpp
    void uci();
};