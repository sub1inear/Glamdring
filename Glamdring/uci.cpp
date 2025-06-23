#include "chess.h"

static void output_to_gui(const char *str, FILE *log) {
    fputs(str, stdout);
    fputs(str, log);
}
static void newline_to_gui(FILE *log) {
    putchar('\n');
    fputc('\n', log);
}

void chess_t::uci() {
    FILE *log = fopen("glamdring.log", "w");

    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // TODO: unify startpos fen in chess_t
    while (true) {
        char input[4096];
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
                        board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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
                move_t move = search();
                output_to_gui("bestmove ", log);
                move.print();
                move.print(log);
                newline_to_gui(log);
            } else if (!strcmp(command, "d")) {
                board.print();
                board.print(log);
            } else if (!strcmp(command, "quit")) {
                return;
            }
            fflush(stdout);
            fflush(log);
        }
    }
}