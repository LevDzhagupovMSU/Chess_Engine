#include "move_interface.hpp"

struct Ai_move : public IMove {
public:
    explicit Ai_move(ce::Move value){
        move = value;
    }

    int from() const{
        return move.from;
    }

    int to() const{
        return move.to;
    }
};
