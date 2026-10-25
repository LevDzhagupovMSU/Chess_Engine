#pragma once

#include "types.hpp"

namespace ce {

struct Move {
    int from;
    int to;
    PieceKind promotion = PieceKind::queen;
};

} // namespace ce
