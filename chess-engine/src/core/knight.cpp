#include "piece.hpp"
#include "board.hpp"
#include "types.hpp"

#include <memory>
#include <string>

namespace ce {

Knight::Knight(int row_, int col_, int price_, Color _color)
    : PieceBase(row_, col_, price_, _color, PieceKind::knight) {}

std::vector<Move> Knight::FINDPOSSIBLEMOVES(const ChessBoard& board) const {
    constexpr int offsets[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    std::vector<Move> moves;
    for (const auto& offset : offsets) {
        addIfPossible(moves, board, row + offset[0], col + offset[1]);
    }
    return moves;
}

bool Knight::ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const {
    if (move.from != board.Index(row, col) || move.to < 0 || move.to >= NUM_SQUARES) return false;
    const int to_row = move.to / BOARD_SIZE;
    const int to_col = move.to % BOARD_SIZE;
    if(board.pieceAt(board.Index(to_row, to_col)) == nullptr || board.pieceAt(board.Index(to_row, to_col))->getColor() != this->getColor()){
        if(((abs(to_row - this->getRow()) == 2) && (abs(to_col - this->getCol()) == 1)) ||
        ((abs(to_row - this->getRow())) == 1 && (abs(to_col - this->getCol()) == 2)))
            return true;
    }
    return false;
}

std::string Knight::getSymbol() const {
    if(this->getColor() == white) return "WN";
    return "BN";
}

} // namespace ce
