#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <limits>
#include <chrono>
#include <utility>
#include <thread>
#include <atomic>
#include <future>
#include <cstdint>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

#include "compat.h"

template <typename T, uint32_t S>
class array_t {
public:
    T data[S];
    uint32_t size; // TODO: replace with end pointer

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
    T *begin() {
        return &data[0];
    }
    T *end() {
        return &data[size];
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
    static constexpr uint32_t max_moves = 218; // https://chess.stackexchange.com/questions/4490/maximum-possible-movement-in-a-turn
    static constexpr int32_t eval_max = INT32_MAX;
    static constexpr int32_t eval_min = -eval_max; // -eval_min with INT32_MIN would overflow
    static constexpr int32_t transposition_table_move_score = 100;

    FILE *log;

    chess_t() {
        log = fopen("glamdring.log", "w");
        if (log == nullptr) {
            // use printf for consistency with opening book failure
            printf("fopen() in chess_t::chess_t() failed: %s", strerror(errno));
            exit(1);
        }
    }

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
            uint64_t zobrist_key;
            uint32_t half_move_clock;
            uint32_t full_moves;
            square_t en_passant;
            bool castling_rights[2][2];
            color_t to_move;
            piece_color_t captured_piece;
        };
        array_t<game_state_t, max_ply> game_state_stack;

        uint32_t last_irrev_ply;

        board_t() {}
        piece_color_t get_piece(square_t square) {
            return board[square];
        }
        void set_piece(square_t square, piece_color_t piece);
        void clear_piece_bitboard(square_t square, piece_color_t piece);
        void clear_piece(square_t square, piece_color_t piece);
        
        uint64_t get_polyglot_key();

        void print(FILE *out = stdout);
        void clear();
        void load_fen(const char *fen);

        void make_move(move_t move);
        void undo_move(move_t move);
    };
    board_t board;

    // move.cpp
    class move_t {
    public:
        /*
        TODO: Add separate bit for castling and perhaps a special value for a null move
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
        
        constexpr move_t(square_t from, square_t to, move_flags_t flags) : from(from), to(to), flags(flags) {}

        move_t(board_t &board, const char *str);
        move_t(board_t &board, uint16_t polyglot_move);

        void compute_flags(board_t &board, move_flags_t promotion, const square_t (&king_castling_end_squares)[][2]);

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
        void print(FILE *out = stdout);
    };
    typedef array_t<move_t, max_moves> move_array_t;

    // transposition_table.cpp
    class transposition_table_t {
    public:
        enum tranposition_type_t : uint8_t {
            EXACT,
            UPPERBOUND,
            LOWERBOUND,
        };
        class transposition_data_t {
        public:
            int32_t eval;
            uint8_t move_idx;
            uint8_t depth;
            tranposition_type_t type;
            friend bool operator ==(const transposition_data_t &a, const transposition_data_t &b) {
                return a.eval == b.eval && a.move_idx == b.move_idx &&
                       a.depth == b.depth && a.type == b.type;
            }
            operator uint64_t() {
                uint64_t result;
                static_assert(sizeof(transposition_data_t) == sizeof(result),
                             "transposition_data_t must be equal in size to uint64_t.");
                memcpy(&result, this, sizeof(result));
                return result;
            }
        };
        struct transposition_entry_t {
            transposition_data_t data;
            uint64_t data_xor_key;
        };
        transposition_entry_t *table;
        static constexpr uint64_t size = 512 * 1024 * 1024; // must be power of two to turn key % size into key & (size - 1)
        static constexpr uint64_t entries = size / sizeof(*table);
        transposition_table_t() {
            table = new transposition_entry_t[entries] {};
        }
        ~transposition_table_t() {
            delete[] table;
        }
        transposition_entry_t lookup(uint64_t key);
        void store(int32_t eval, uint8_t move_idx, uint64_t key, int32_t alpha, int32_t beta, uint8_t depth);
    };
    transposition_table_t transposition_table;

    // opening_book.cpp
    class opening_book_t {
    public:
        FILE *book;

        int32_t last;
        
        uint32_t seed;

        class polyglot_entry_t {
        public:
            uint64_t zobrist_key;
            uint16_t move;
            uint16_t weight;
            uint16_t learn;
            void byteswap_key();
            void byteswap_non_key();
        };
        opening_book_t() {
            seed = (uint32_t)std::chrono::steady_clock::now().time_since_epoch().count();
        }
        uint32_t random();
        bool set_book(const char *filename);
        void parse_polyglot_move(uint16_t move);
        bool lookup(board_t &board, move_t &best_move);
    };
    opening_book_t opening_book;

    // movegen.cpp
    static void serialize_bitboard(square_t square, uint64_t moves_bitboard, uint64_t enemies, move_array_t &moves);
    template <chess_t::color_t to_move>
    void gen_pawn_moves(uint64_t pawns, uint64_t blockers, uint64_t allies, uint64_t enemies, uint64_t legal, uint64_t (&pin_lines)[64], move_array_t &moves);
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
    void gen_pins(uint64_t (&pin_lines)[64], square_t square, uint64_t allies, uint64_t enemies); // TODO: use reference?
    move_array_t gen_moves();
    
    // eval.cpp
    template <color_t color>
    int32_t count_material();
    int32_t eval();

    // draw.cpp
    bool is_repetition();
    bool is_insufficient_material();
    bool is_fifty_move_rule();

    // search.cpp
    move_t best_move;
    uint64_t nodes;

    std::atomic<bool> searching;
    move_t order_moves(move_array_t &moves, uint8_t (&scores)[max_moves], uint32_t idx);
    int32_t negamax(uint32_t depth, uint64_t max_nodes, bool root = true, int32_t alpha = eval_min, int32_t beta = eval_max);
    int32_t search(uint32_t max_depth, uint64_t max_nodes, bool use_opening_book = true);
    int32_t search_timed(std::chrono::milliseconds time, uint32_t max_depth, uint64_t max_nodes, bool use_opening_book = true);
    void stop_search();

    // timeman.cpp
    std::chrono::milliseconds get_search_time(std::chrono::milliseconds time, std::chrono::milliseconds inc, std::chrono::milliseconds input_move_time);

    // uci.cpp
    void log_uci(const char *fmt, ...);
    void flush_uci();
    void print_uci(const char *fmt, ...);

    struct go_options_t {
        std::chrono::milliseconds time[2];
        std::chrono::milliseconds inc[2];
               
        bool infinite;

        uint32_t max_depth;
        uint32_t max_nodes;
        std::chrono::milliseconds input_move_time;
    };

    go_options_t parse_go_command();
    void parse_position_command();
    int32_t search_uci(std::chrono::milliseconds time, bool infinite, uint32_t max_depth, uint64_t max_nodes);
    void uci();

    // precomp.cpp
    static void gen_precomp_data();

    // test.cpp
    uint64_t perft(uint32_t depth, bool root = true);
    void test_movegen();
    void test_transposition_table();
    void test_draw();

};