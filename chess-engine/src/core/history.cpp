#include "history.hpp"

namespace ce {

void History::record(PieceKind piece, Move move, Color color, bool captured) {
    records_.push_back({piece, move, color, captured});
}

void History::popLast() noexcept {
    records_.pop_back();
}

void History::clear() noexcept {
    records_.clear();
}

bool History::empty() const noexcept {
    return records_.empty();
}

std::size_t History::size() const noexcept {
    return records_.size();
}

const MoveRecord& History::last() const {
    return records_.back();
}

const MoveRecord& History::at(std::size_t index) const {
    return records_.at(index);
}

History::const_iterator History::begin() const noexcept {
    return records_.begin();
}

History::const_iterator History::end() const noexcept {
    return records_.end();
}

} // namespace ce
