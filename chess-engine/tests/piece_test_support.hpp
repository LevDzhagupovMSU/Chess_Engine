#pragma once

#include "board.hpp"

#include <memory>
#include <stdexcept>
#include <type_traits>

namespace test {

inline int square(char file, int rank) {
    return (8 - rank) * ce::BOARD_SIZE + (file - 'a');
}

inline ce::Move move(char from_file, int from_rank, char to_file, int to_rank) {
    return {square(from_file, from_rank), square(to_file, to_rank)};
}

inline ce::ChessBoard emptyBoard() {
    ce::ChessBoard board;
    board.clear();
    return board;
}

template <typename T>
T& put(ce::ChessBoard& board, char file, int rank, ce::Color color) {
    auto piece = std::make_unique<T>(8 - rank, file - 'a', 100, color);
    T& result = *piece;
    board.setPiece(8 - rank, file - 'a', std::move(piece));
    return result;
}

inline bool canMove(const ce::ChessBoard& board, ce::Move move) {
    const ce::Piece* piece = board.pieceAt(move.from);
    return piece && piece->ISPOSSIBLEMOVE(move, board);
}

inline void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

inline void play(ce::ChessBoard& board, ce::Move move) {
    require(board.makeMove(move), "Expected legal move is unavailable");
}

} // namespace test
