#include "move_interface.hpp"

struct bad_move : public IMove {
public:
    bad_move() {
        move = {-1, -1};
    }

    int from() const{
        return -1;
    }

    int to() const{
        return -1;
    }
};
