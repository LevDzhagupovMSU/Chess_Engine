#include "piece.hpp"
#include "board.hpp"
#include "types.hpp"

#include <memory>
#include <string>

namespace ce {

Rook::Rook(int row_, int col_, int price_, Color _color)
    : PieceBase(row_, col_, price_, _color, PieceKind::rook) {}

std::vector<Move> Rook::FINDPOSSIBLEMOVES(const ChessBoard& board) const {
    std::vector<Move> moves;
    addRay(moves, board, -1, 0);
    addRay(moves, board, 1, 0);
    addRay(moves, board, 0, -1);
    addRay(moves, board, 0, 1);
    return moves;
}

bool Rook::ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const {
    if (move.from != board.Index(row, col) || move.to < 0 || move.to >= NUM_SQUARES) return false;
    const int to_row = move.to / BOARD_SIZE;
    const int to_col = move.to % BOARD_SIZE;
    if(to_row == row && to_col == col) return false;
    if(board.pieceAt(board.Index(to_row, to_col)) != nullptr && board.pieceAt(board.Index(to_row, to_col))->getColor() == color) return false;
    if(to_row == this->getRow() && to_col != this->getCol()){
        int to_index = board.Index(to_row, to_col), index = board.Index(this->getRow(), this->getCol()), direction = (to_index > index) ? 1 : -1;
        for(int i = 1; i < abs(to_index - index); i++){
            if(board.pieceAt(index + direction * i) != nullptr) return false;
        }
        return true;
    }
    if(to_row != this->getRow() && to_col == this->getCol()){
        int to_index = board.Index(to_row, to_col), index = board.Index(this->getRow(), this->getCol()), direction = (to_index > index) ? 8 : -8;
        for(int i = 1; i < abs(to_index - index) / 8; i++){
            if(board.pieceAt(index + direction * i) != nullptr) return false;
        }
        return true;
    }
    return false;
}

std::string Rook::getSymbol() const {
    if(this->getColor() == white) return "WR";
    return "BR";
}

} // namespace ce
