#include "piece.hpp"
#include "board.hpp"
#include "types.hpp"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ce {

PieceBase::PieceBase(int row_, int col_, int price_, Color color_, PieceKind kind_)
    : row(row_), col(col_), price(price_), color(color_), kind(kind_) {}

int PieceBase::getRow() const { return row; }
int PieceBase::getCol() const { return col; }
int PieceBase::getPrice() const { return price; }
Color PieceBase::getColor() const { return color; }
PieceKind PieceBase::getKind() const { return kind; }

void PieceBase::setPosition(int new_row, int new_col) {
    row = new_row;
    col = new_col;
}

void PieceBase::addIfPossible(std::vector<Move>& moves, const ChessBoard& board,
                              int to_row, int to_col) const {
    if (to_row < 0 || to_row >= BOARD_SIZE || to_col < 0 || to_col >= BOARD_SIZE) {
        return;
    }
    Move move{board.Index(row, col), board.Index(to_row, to_col)};
    if (ISPOSSIBLEMOVE(move, board)) moves.push_back(move);
}

void PieceBase::addRay(std::vector<Move>& moves, const ChessBoard& board,
                       int row_step, int col_step) const {
    for (int to_row = row + row_step, to_col = col + col_step;
         to_row >= 0 && to_row < BOARD_SIZE && to_col >= 0 && to_col < BOARD_SIZE;
         to_row += row_step, to_col += col_step) {
        addIfPossible(moves, board, to_row, to_col);
        if (board.pieceAt(board.Index(to_row, to_col))) break;
    }
}

std::unique_ptr<Piece> makePiece(PieceKind kind, int row, int col, int price, Color color) {
    switch (kind) {
    case PieceKind::pawn: return std::make_unique<Pawn>(row, col, price, color);
    case PieceKind::knight: return std::make_unique<Knight>(row, col, price, color);
    case PieceKind::bishop: return std::make_unique<Bishop>(row, col, price, color);
    case PieceKind::rook: return std::make_unique<Rook>(row, col, price, color);
    case PieceKind::queen: return std::make_unique<Queen>(row, col, price, color);
    case PieceKind::king: return std::make_unique<King>(row, col, price, color);
    }
    throw std::invalid_argument("Unknown piece kind");
}

} // namespace ce
