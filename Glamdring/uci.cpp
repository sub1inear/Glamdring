#include "chess.h"
#include "data.h"

static void output_to_gui(const char *fmt, FILE *log, ...) {
    va_list args;
    va_start(args, log);

    vprintf(fmt, args);
    vfprintf(log, fmt, args);

    va_end(args);

    fflush(stdout);
    fflush(log);
}

chess_t::go_options_t chess_t::parse_go_command() {
    go_options_t go_options = {
        { (std::chrono::milliseconds)0, (std::chrono::milliseconds)0, },
        { (std::chrono::milliseconds)0, (std::chrono::milliseconds)0, },
        false,
        UINT32_MAX,
        UINT32_MAX,
        (std::chrono::milliseconds)0,
    };

    bool empty = true;

    while (char *option = strtok(nullptr, " ")) {
        if (!strcmp(option, "wtime")) {
            go_options.time[WHITE] = (std::chrono::milliseconds)atoll(strtok(nullptr, " "));
        } else if (!strcmp(option, "btime")) {
            go_options.time[BLACK] = (std::chrono::milliseconds)atoll(strtok(nullptr, " "));
        } else if (!strcmp(option, "winc")) {
            go_options.inc[WHITE] = (std::chrono::milliseconds)atoll(strtok(nullptr, " "));
        } else if (!strcmp(option, "binc")) {
            go_options.inc[BLACK] = (std::chrono::milliseconds)atoll(strtok(nullptr, " "));
        } else if (!strcmp(option, "depth")) {
            go_options.max_depth = atoi(strtok(nullptr, " "));
        } else if (!strcmp(option, "nodes")) {
            go_options.max_nodes = atoi(strtok(nullptr, " "));
        } else if (!strcmp(option, "movetime")) {
            go_options.input_move_time = (std::chrono::milliseconds)atoll(strtok(nullptr, " "));
        } else if (!strcmp(option, "infinite")) {
            go_options.infinite = true;
        }
        empty = false;
    }

    if (empty) {
        go_options.infinite = true;
    }
    return go_options;
}

void chess_t::parse_position_command() {
    char *position = strtok(nullptr, " ");
    char *args = strtok(nullptr, "");
    char *moves = args ? strstr(args, "moves") : nullptr;
    if (position) {
        if (!strcmp(position, "startpos")) {
            board.load_fen(data::startpos_fen);
        } else if (!strcmp(position, "fen")) {
            if (moves) {
                // null delimit args
                *(moves - 1) = '\0';
            }
            if (args) {
                board.load_fen(args);
            }    
        }
        if (moves) {
            char *move = strtok(moves + sizeof("moves"), " ");
            while (move) {
                board.make_move({ board, move });
                move = strtok(nullptr, " ");
            }
        }
    }
}

int32_t chess_t::search_uci(std::chrono::milliseconds time, bool infinite, uint32_t max_depth, uint64_t max_nodes, FILE *log) {
    int32_t eval = infinite ? search(max_depth, max_nodes, false) : search_timed(time, max_depth, max_nodes);
    
    output_to_gui("bestmove ", log);
    best_move.print();
    best_move.print(log);
    output_to_gui("\n"
                  "info string %d nodes searched\n",
                  log,
                  nodes
    );

    return eval;
}

void chess_t::uci() {
    // TODO: make log part of chess_t
    FILE *log = fopen("glamdring.log", "w");

    if (!opening_book.set_book("../../OpeningBooks/Titans.bin")) {
        output_to_gui("info string set_opening_book() failed: %s\n", log, strerror(errno));
        fflush(log);
        return;
    }

    board.load_fen(data::startpos_fen);
    while (true) {
        char input[64 * 1024];
        gets_s(input);
        
        fprintf(log, "%s\n", input);
        fflush(log);

        char *command = strtok(input, " ");
        if (command) {
            if (!strcmp(command, "uci")) {
                output_to_gui("id name Glamdring\n"
                              "id author sublinear\n"
                              "uciok\n", log);
            } else if (!strcmp(command, "isready")) {
                output_to_gui("readyok\n", log);
            } else if (!strcmp(command, "stop")) {
                stop_search();
            } else if (!strcmp(command, "quit")) {
                stop_search();
                return;
            } else {
                if (searching) {
                    continue;
                }
                if (!strcmp(command, "go")) {
                    go_options_t go_options = parse_go_command();

                    color_t to_move = board.game_state_stack.last()->to_move;
                    std::chrono::milliseconds move_time = get_search_time(go_options.time[to_move], go_options.inc[to_move], go_options.input_move_time);

                    if (move_time > (std::chrono::milliseconds)0) {
                        output_to_gui("info string searching for %d ms\n", log, move_time);
                    }

                    std::thread search_thread { &chess_t::search_uci, this, move_time, go_options.infinite, go_options.max_depth, go_options.max_nodes, log };
                    search_thread.detach();
                } else if (!strcmp(command, "position")) {
                    parse_position_command();
                } else if (!strcmp(command, "d")) {
                    board.print();
                    board.print(log);
                } else if (!strcmp(command, "eval")) {
                    output_to_gui("%d\n", log, eval());
                }
            }
        }
    }
}