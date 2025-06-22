#include "chess.h"

void chess_t::uci() {
    while (true) {
        char input[256];
        gets_s(input);
        char *command = strtok(input, " ");

        if (command) {
            if (!strcmp(command, "uci")) {
                puts("uciok");
            } else if (!strcmp(command, "isready")) {
                puts("readyok");
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
                puts("bestmove e7e5");
            } else if (!strcmp(command, "d")) {
                board.print();
            } else if (!strcmp(command, "quit")) {
                return;
            }
        }
    }
}