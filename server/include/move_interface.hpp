#pragma once

#include <iostream>
#include "move.hpp"

struct IMove {
    ce::Move move{0, 0};

    virtual int from() const =0;
    virtual int to() const =0;
    virtual ~IMove() = default;
};
