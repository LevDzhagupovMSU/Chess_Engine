#include "piece.hpp"
#include "board.hpp"
#include "types.hpp"

#include <memory>
#include <string>

namespace ce {

King::King(int row_, int col_, int price_, Color _color)
    : PieceBase(row_, col_, price_, _color, PieceKind::king) {}

std::vector<Move> King::FINDPOSSIBLEMOVES(const ChessBoard& board) const {
    std::vector<Move> moves;
    for (int row_step = -1; row_step <= 1; ++row_step) {
        for (int col_step = -1; col_step <= 1; ++col_step) {
            if (row_step != 0 || col_step != 0) {
                addIfPossible(moves, board, row + row_step, col + col_step);
            }
        }
    }
    addIfPossible(moves, board, row, col - 2);
    addIfPossible(moves, board, row, col + 2);
    return moves;
}

bool King::ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const {
    if (move.from != board.Index(row, col) || move.to < 0 || move.to >= NUM_SQUARES) return false;
    const int to_row = move.to / BOARD_SIZE;
    const int to_col = move.to % BOARD_SIZE;
    if(board.pieceAt(move.to) && board.pieceAt(move.to)->getColor() == getColor()) return false;
    const int row_distance = abs(to_row - row);
    const int col_distance = abs(to_col - col);

    if (row_distance == 0 && col_distance == 2) {
        const int home_row = getColor() == white ? 7 : 0;
        if (row != home_row || col != 4) return false;

        const int direction = to_col > col ? 1 : -1;
        const int rook_index = board.Index(row, direction == 1 ? 7 : 0);
        const Piece* rook = board.pieceAt(rook_index);
        if (!rook || rook->getKind() != PieceKind::rook || rook->getColor() != getColor() ||
            !board.isItPossibleToCastle(getColor(), rook_index) || board.isCheck(getColor())) {
            return false;
        }

        for (int square = move.from + direction; square != rook_index; square += direction) {
            if (board.pieceAt(square)) return false;
        }

        const Color opponent = getColor() == white ? black : white;
        return !board.isSquaredAttacked(move.from + direction, opponent) &&
               !board.isSquaredAttacked(move.to, opponent);
    }

    return row_distance <= 1 && col_distance <= 1 &&
           (row_distance != 0 || col_distance != 0);
}

std::string King::getSymbol() const {
    if(this->getColor() == white) return "WK";
    return "BK";
}

} // namespace ce
