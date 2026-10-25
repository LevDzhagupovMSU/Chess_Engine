#include "piece.hpp"
#include "board.hpp"
#include "types.hpp"

#include <memory>
#include <string>

namespace ce {

Bishop::Bishop(int row_, int col_, int price_, Color _color)
    : PieceBase(row_, col_, price_, _color, PieceKind::bishop) {}

std::vector<Move> Bishop::FINDPOSSIBLEMOVES(const ChessBoard& board) const {
    std::vector<Move> moves;
    for (int row_step : {-1, 1}) {
        for (int col_step : {-1, 1}) {
            addRay(moves, board, row_step, col_step);
        }
    }
    return moves;
}

bool Bishop::ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const {
    if (move.from != board.Index(row, col) || move.to < 0 || move.to >= NUM_SQUARES) return false;
    const int to_row = move.to / BOARD_SIZE;
    const int to_col = move.to % BOARD_SIZE;
    if(to_row == row || to_col == col) return false;
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
    return false;
}

std::string Bishop::getSymbol() const {
    if(this->getColor() == white) return "WB";
    return "BB";
}

} // namespace ce
