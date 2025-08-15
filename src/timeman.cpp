#include "chess.h"

std::chrono::milliseconds 
chess_t::get_search_time(std::chrono::milliseconds time, 
                         std::chrono::milliseconds inc,
                         std::chrono::milliseconds input_move_time) {
        // https://www.chessprogramming.org/Getting_Started#Search_and_Evaluation
        std::chrono::milliseconds calc_move_time { time / 20 + inc / 2 };
                    
        std::chrono::milliseconds move_time;

        if (input_move_time > (std::chrono::milliseconds)0 &&
            calc_move_time > (std::chrono::milliseconds)0) {
            move_time = std::min(calc_move_time, input_move_time);
        } else if (input_move_time == (std::chrono::milliseconds)0) {
            move_time = calc_move_time;
        } else if (calc_move_time == (std::chrono::milliseconds)0) {
            move_time = input_move_time;
        }

        return move_time;
}