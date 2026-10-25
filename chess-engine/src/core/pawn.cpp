#include "piece.hpp"
#include "board.hpp"
#include "types.hpp"

#include <memory>
#include <string>

namespace ce {

Pawn::Pawn(int row_, int col_, int price_, Color _color)
    : PieceBase(row_, col_, price_, _color, PieceKind::pawn) {}

bool Pawn::ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const {
    if (move.from != board.Index(row, col) || move.to < 0 || move.to >= NUM_SQUARES) return false;
    const int to_row = move.to / BOARD_SIZE;
    const int to_col = move.to % BOARD_SIZE;
    const int promotion_row = getColor() == white ? 0 : 7;
    if (to_row == promotion_row &&
        move.promotion != PieceKind::queen &&
        move.promotion != PieceKind::rook &&
        move.promotion != PieceKind::bishop &&
        move.promotion != PieceKind::knight) {
        return false;
    }
    int direction = (this->getColor() == white) ? -1 : 1;
    int index = board.Index(to_row, to_col);
    if(to_col == this->getCol() && to_row == this->getRow() + direction && board.pieceAt(index) == nullptr) return true;
    const int start_row = getColor() == white ? 6 : 1;
    if(to_col == this->getCol() && to_row == this->getRow() + 2 * direction &&
       row == start_row && board.pieceAt(index) == nullptr){
        if(board.pieceAt(board.Index(this->getRow() + direction, to_col)) == nullptr) return true;
    }
    if(abs(to_col - this->getCol()) == 1 && to_row == this->getRow() + direction){
        if(board.pieceAt(index) != nullptr) {
            return board.pieceAt(index)->getColor() != getColor();
        }
        return board.isEnPassant(move);
    }
    return false;
}

std::vector<Move> Pawn::FINDPOSSIBLEMOVES(const ChessBoard& board) const {
    std::vector<Move> moves;
    const int direction = getColor() == white ? -1 : 1;
    addIfPossible(moves, board, row + direction, col);
    addIfPossible(moves, board, row + 2 * direction, col);
    addIfPossible(moves, board, row + direction, col - 1);
    addIfPossible(moves, board, row + direction, col + 1);

    const int promotion_row = getColor() == white ? 0 : 7;
    std::vector<Move> extra;
    for (const Move& move : moves) {
        if (move.to / BOARD_SIZE == promotion_row) {
            extra.push_back({move.from, move.to, PieceKind::rook});
            extra.push_back({move.from, move.to, PieceKind::bishop});
            extra.push_back({move.from, move.to, PieceKind::knight});
        }
    }
    moves.insert(moves.end(), extra.begin(), extra.end());
    return moves;
}

std::string Pawn::getSymbol() const {
    if(this->getColor() == white) return "WP";
    return "BP";
}

} // namespace ce
