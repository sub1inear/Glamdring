#include "chess.h"
#include "data.h"

template <typename ...A>
void chess_t::log_uci(const char *str, A... args) {
    fprintf(log, str, args...);
    fflush(log);
}
void chess_t::flush_uci() {
    fflush(stdout);
    fflush(log);
}

void chess_t::print_uci(const char *str) {
    fputs(str, stdout);
    fputs(str, log);
    flush_uci();
}

template <typename ...A>
void chess_t::print_uci(const char *fmt, A... args) {
    printf(fmt, args...);
    fprintf(log, fmt, args...);
    flush_uci();
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
    
    print_uci("bestmove ");
    best_move.print();
    best_move.print(log);
    print_uci("\n"
               "info string %d nodes searched\n",
               nodes
    );

    return eval;
}

void chess_t::uci() {
    if (!opening_book.set_book("../../OpeningBooks/Titans.bin")) {
        print_uci("info string set_opening_book() failed: %s\n", strerror(errno));
        return;
    }

    board.load_fen(data::startpos_fen);
    while (true) {
        char input[64 * 1024];
        gets_s(input);
        
        log_uci("%s\n", input);

        char *command = strtok(input, " ");
        if (command) {
            if (!strcmp(command, "uci")) {
                print_uci("id name Glamdring\n"
                          "id author sublinear\n"
                          "uciok\n");
            } else if (!strcmp(command, "isready")) {
                print_uci("readyok\n");
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
                        print_uci("info string searching for %d ms\n", move_time);
                    }

                    std::thread search_thread { &chess_t::search_uci, this, move_time, go_options.infinite, go_options.max_depth, go_options.max_nodes, log };
                    search_thread.detach();
                } else if (!strcmp(command, "position")) {
                    parse_position_command();
                } else if (!strcmp(command, "d")) {
                    board.print();
                    board.print(log);
                } else if (!strcmp(command, "eval")) {
                    print_uci("%d\n", eval());
                }
            }
        }
    }
}