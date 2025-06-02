#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <random>
#include <cctype>
#include <cassert>
#include <cstdlib>

class chess_t {
public:
    typedef int32_t square_t;

    enum color_t : uint32_t {
        WHITE,
        BLACK
    };
    enum piece_t : uint32_t {
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING,
        CLEAR
    };
    enum castling_side_t : uint32_t {
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
        piece_color_t board[64];
        // TODO: statically allocate
        std::vector<piece_square_t> pieces;
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
            bitboards[piece.color][piece.piece] |= (uint64_t)1 << (63 - square);
            pieces.push_back({piece, square});
        }
        void clear_piece(square_t square, piece_color_t piece) {
            board[square] = { CLEAR, WHITE };
            // assumes piece is there
            bitboards[piece.color][piece.piece] ^= (uint64_t)1 << (63 - square);
            // assumes no overlapping squares
            for (std::vector<piece_square_t>::iterator iter = pieces.begin(); iter != pieces.end(); iter++) {
                if (iter->square == square) {
                    pieces.erase(iter);
                    break;
                }
            }
        }
        void set_to_move(color_t color) {
            to_move = color;
        }
        void set_castling_rights(color_t color, castling_side_t side, bool enable) {
            castling_rights[color][side] = enable;
        }
        void set_en_passant(square_t square) {
            en_passant = square;
        }
        void print_board();
        void clear();
        void load_fen(const char *fen);
    } board;
    
    // utils.cpp
    static square_t file_rank_to_square(square_t file, square_t rank);
    static void square_to_file_rank(square_t square, char *out);
    
    void print_bitboard(uint64_t bitboard);
    uint64_t magic_bitboard_key(uint64_t mask, uint64_t blockers, uint64_t magic, int shift);
    
    void gen_magics();  
    int test();

    

};