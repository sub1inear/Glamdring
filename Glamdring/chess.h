#pragma once
#define _CRT_SECURE_NO_WARNINGS
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

    static constexpr square_t null_square = -1;
    
    // board.cpp
    class board_t {
    private:
        alignas(64)
        piece_color_t board[64];
        // TODO: statically allocate
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
    uint64_t gen_rook_moves(square_t square, uint64_t blockers) {
        magic_t magic = rook_magics[square];
        uint64_t key = (blockers & magic.mask) * magic.magic >> magic.shift;
        return magic_move_data[magic.idx + key];
    }
    uint64_t gen_bishop_moves(chess_t::square_t square, uint64_t blockers) {
        magic_t magic = bishop_magics[square];
        uint64_t key = (blockers & magic.mask) * magic.magic >> magic.shift;
        return magic_move_data[magic.idx + key];
    }
    uint64_t gen_queen_moves(chess_t::square_t square, uint64_t blockers) {
        return gen_rook_moves(square, blockers) | gen_bishop_moves(square, blockers);
    }
    // uint64_t gen_knight_moves(square_t square);
    // uint64_t gen_king_moves(square_t square);
    // static void serialize_moves(uint64_t bitboard, array_t<move_t moves, max_moves> &moves);    

    // precomp.cpp
    static void gen_magics();

    // utils.cpp
    static square_t file_rank_to_square(square_t file, square_t rank);
    static void square_to_file_rank(square_t square, char *out);
    static void print_bitboard(uint64_t bitboard);
    int test();
};