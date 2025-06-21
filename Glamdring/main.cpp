#include "chess.h"
#include "data.h"
#include <iostream>

int main(int argc, char **argv) {
    chess_t chess;
    
    uint32_t failures = chess.test();
    if (failures == 0) {
        std::cout << "\x1b[32mAll tests successful!\x1b[0m";
    } else {
        std::cout << "\x1b[31m" << failures << " tests failed.\x1b[0m";
        return 1;
    }

}