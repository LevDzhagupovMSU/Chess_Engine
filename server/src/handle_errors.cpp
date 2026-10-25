#include "session.hpp"

void Session::error_handle(error_type error){
    switch (error) {
        case error_type::lost_connection:
            stop();
            break;
        case error_type::bad_move:
            send_bad_move();
            break;
    }
}

void Session::end_handle(game_end end_type){
    switch (end_type) {
        case game_end::win:
            send_result("win");
            break;
        case game_end::lost:
            send_result("lost");
            break;
        case game_end::draw:
            send_result("draw");
            break;
        case game_end::stalemate:
            send_result("stalemate");
            break;
    }
}