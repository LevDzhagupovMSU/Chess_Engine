#include "move_interface.hpp"

struct Player_move : public IMove {
public:
    explicit Player_move(ce::Move value){
        move = value;
    }

    int from() const{
        return move.from;
    }

    int to() const{
        return move.to;
    }
};
