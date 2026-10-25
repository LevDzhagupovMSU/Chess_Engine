#include "piece.hpp"
#include "board.hpp"
#include "types.hpp"

#include <memory>
#include <string>

namespace ce {

Queen::Queen(int row_, int col_, int price_, Color _color)
    : PieceBase(row_, col_, price_, _color, PieceKind::queen) {}

std::vector<Move> Queen::FINDPOSSIBLEMOVES(const ChessBoard& board) const {
    std::vector<Move> moves;
    addRay(moves, board, -1, 0);
    addRay(moves, board, 1, 0);
    addRay(moves, board, 0, -1);
    addRay(moves, board, 0, 1);
    for (int row_step : {-1, 1}) {
        for (int col_step : {-1, 1}) {
            addRay(moves, board, row_step, col_step);
        }
    }
    return moves;
}

bool Queen::ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const {
    if (move.from != board.Index(row, col) || move.to < 0 || move.to >= NUM_SQUARES) return false;
    const int to_row = move.to / BOARD_SIZE;
    const int to_col = move.to % BOARD_SIZE;
    if(board.pieceAt(board.Index(to_row, to_col)) != nullptr && board.pieceAt(board.Index(to_row, to_col))->getColor() == this->getColor()) return false;
    if(abs(to_row - this->getRow()) == abs(to_col - this->getCol())){
        int checkRow = (to_row > this->getRow()) ? 1 : -1;
        int checkCol = (to_col > this->getCol()) ? 1 : -1;
        int diff = abs(to_row - this->getRow());
        for(int i = 1; i < diff; i++){
            if(board.pieceAt(board.Index(this->getRow() + checkRow * i, this->getCol() + checkCol * i)) != nullptr) return false;
        }
        return true;
    }
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

std::string Queen::getSymbol() const {
    if(this->getColor() == white) return "WQ";
    return "BQ";
}

} // namespace ce
