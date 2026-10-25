#pragma once

#include "types.hpp"
#include "move.hpp"
#include <cstddef>
#include <vector>

namespace ce {

struct MoveRecord {
    PieceKind piece;
    Move move;
    Color color;
    bool captured = false;
};

class History {
    std::vector<MoveRecord> records_;
public:
    using const_iterator = std::vector<MoveRecord>::const_iterator;

    void record(PieceKind piece, Move move, Color color, bool captured = false);
    void popLast() noexcept;
    void clear() noexcept;
    bool empty() const noexcept;
    std::size_t size() const noexcept;
    const MoveRecord& last() const;
    const MoveRecord& at(std::size_t index) const;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
};

} // namespace ce
