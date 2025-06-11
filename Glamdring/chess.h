#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <cctype>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <immintrin.h>
#include <omp.h>
#include "data.h"
#pragma comment(lib, "libomp.lib")

template <typename T, uint32_t S>
class array_t {
private:
    T data[S];
    uint32_t size;
public:
    template <typename... U>
    array_t(U... array): data(array...), size(sizeof...(U)) {};
    void add(T item) {
        data[size++] = item;
    }
    void remove(uint32_t idx) {
        size--;
        memmove(data + idx, data + idx + 1, (size - idx) * sizeof(T));
    }
    uint32_t get_size() {
        return size;
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
    // knight, bishop, rook, and queen map to move_flags_t
    enum piece_t : uint8_t {
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        PAWN,
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
    // TODO: use 1 byte
    struct piece_color_t {
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
        move_t(uint16_t value) {
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
        piece_t get_promotion() {
            return (piece_t)(flags & 0x7);
        }

    };

    static constexpr square_t null_square = -1;
    
    // board.cpp
    class board_t {
    private:
        alignas(64)
        piece_color_t board[64];
        array_t<piece_square_t, 64> pieces;
        uint64_t bitboards[2][6];

        color_t to_move;
        bool castling_rights[2][2];
        uint32_t half_move_clock;
        uint32_t full_moves;
        square_t en_passant;
        
    public:
        piece_color_t get_piece(square_t square) {
            return board[square];
        }
        void set_piece(square_t square, piece_color_t piece) {
            board[square] = piece;
            bitboards[piece.color][piece.piece] |= 1ull << (64 - square);
            pieces.add({piece, square});
        }
        void clear_piece(square_t square, piece_color_t piece) {
            if (board[square].piece == CLEAR) {
                return;
            }
            board[square] = { CLEAR, WHITE };
            bitboards[piece.color][piece.piece] ^= 1ull << (64 - square);
            // assumes no overlapping pieces
            for (uint32_t i = 0; i < 64; i++) {
                if (pieces[i].square == square) {
                    pieces.remove(i);
                    break;
                }
            }
        }
        void print();
        void clear();
        void load_fen(const char *fen);
    } board;
    
    // movegen.cpp
    uint64_t gen_rook_moves(square_t square, uint64_t blockers, uint64_t allies) {
        magic_t magic = rook_magics[square];
        uint64_t key = (blockers & magic.mask) * magic.magic >> magic.shift;
        uint64_t moves = magic_move_data[magic.idx + key];
        moves &= ~allies;
        return moves;
    }
    uint64_t gen_bishop_moves(square_t square, uint64_t blockers, uint64_t allies) {
        magic_t magic = rook_magics[square];
        uint64_t key = (blockers & magic.mask) * magic.magic >> magic.shift;
        uint64_t moves = magic_move_data[magic.idx + key];
        moves &= ~allies;
        return moves;
    }
    uint64_t gen_queen_moves(square_t square, uint64_t blockers, uint64_t allies) {
        return gen_rook_moves(square, blockers, allies) | gen_bishop_moves(square, blockers, allies);
    }
    uint64_t gen_knight_moves(square_t square) {
        return knight_move_data[square];
    };
    uint64_t gen_king_moves(square_t square) {
        return king_move_data[square];
    };
    // static void serialize_moves(uint64_t bitboard, array_t<move_t moves, max_moves> &moves);    
    
    // precomp.cpp
    static void gen_precomp_data();

    // utils.cpp
    static square_t file_rank_to_square(square_t file, square_t rank);
    static void square_to_file_rank(square_t square, char *out);
    static void print_bitboard(uint64_t bitboard);
    int test();
};