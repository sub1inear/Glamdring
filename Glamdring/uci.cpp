#include "chess.h"
#include "data.h"

static void output_to_gui(const char *fmt, FILE *log, ...) {
    va_list args;
    va_start(args, log);

    vprintf(fmt, args);
    vfprintf(log, fmt, args);

    va_end(args);
}
static void newline_to_gui(FILE *log) {
    putchar('\n');
    fputc('\n', log);
}

int32_t chess_t::search_uci(std::chrono::milliseconds time, bool infinite, FILE *log) {

    int32_t eval = infinite ? search() : search_timed(time);
    
    output_to_gui("bestmove ", log);
    best_move.print();
    best_move.print(log);
    newline_to_gui(log);

    fflush(stdout);
    fflush(log);

    return eval;
}

void chess_t::uci() {
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

        char *command = strtok(input, " ");
        if (command) {
            if (!strcmp(command, "uci")) {
                output_to_gui("uciok\n", log);
            } else if (!strcmp(command, "isready")) {
                output_to_gui("readyok\n", log);
            } else if (!strcmp(command, "position")) {
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
            } else if (!strcmp(command, "go")) {

                int32_t time[2] { 0, 0 };
                int32_t inc[2] { 0, 0 };
               
                bool infinite = true;

                while (char *option = strtok(nullptr, " ")) {
                    if (!strcmp(option, "wtime")) {
                        time[WHITE] = atoi(strtok(nullptr, " "));
                        infinite = false;
                    } else if (!strcmp(option, "btime")) {
                        time[BLACK] = atoi(strtok(nullptr, " "));
                        infinite = false;
                    } else if (!strcmp(option, "winc")) {
                        inc[WHITE] = atoi(strtok(nullptr, " "));
                    } else if (!strcmp(option, "binc")) {
                        inc[BLACK] = atoi(strtok(nullptr, " "));
                    }
                }

                color_t to_move = board.game_state_stack.last()->to_move;
                // https://www.chessprogramming.org/Getting_Started#Search_and_Evaluation
                std::chrono::milliseconds move_time { time[to_move] / 20 + inc[to_move] / 2 };

                output_to_gui("info string searching for %d ms\n", log, move_time);
                
                std::thread search_thread { &chess_t::search_uci, this, move_time, infinite, log };
                search_thread.detach();
            } else if (!strcmp(command, "stop")) {
                stop_search();
            } else if (!strcmp(command, "d")) {
                board.print();
                board.print(log);
            } else if (!strcmp(command, "eval")) {
                output_to_gui("%d\n", log, eval()); 
            } else if (!strcmp(command, "quit")) {
                fclose(log);
                return;
            }
            fflush(stdout);
            fflush(log);
        }
    }
}