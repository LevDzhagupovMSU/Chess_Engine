#include "board.hpp"
#include "piece.hpp"
#include "types.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>

namespace ce {

bool ChessBoard::isCheck(Color _color) const {
    const int kingIndex = kingSquare(_color);
    return isSquaredAttacked(kingIndex, _color == white ? black : white);
}

bool ChessBoard::isCheckMate(Color _color) {
    if (!isCheck(_color)) { 
        return false;
    }
    return !hasAnyLegalMove(_color);
}

bool ChessBoard::hasAnyLegalMove(Color _color) {
    for(int i = 0; i < NUM_SQUARES; i++) {
        if(pieces_[i] && pieces_[i]->getColor() == _color) {
            auto moves = pieces_[i]->FINDPOSSIBLEMOVES(*this);
            for(const auto& move : moves) {
                if (isLegalMove(move)) return true;
            }
        }
    }
    return false;
}

bool ChessBoard::wouldSafeKing(Move move) {
    return isLegalMove(move);
}

bool ChessBoard::isLegalMove(Move move) {
    if (move.from < 0 || move.from >= NUM_SQUARES ||
        move.to < 0 || move.to >= NUM_SQUARES ||
        !pieces_[move.from] || !pieces_[move.from]->ISPOSSIBLEMOVE(move, *this)) {
        return false;
    }
    if (pieces_[move.to] && pieces_[move.to]->getKind() == PieceKind::king) {
        return false;
    }
    Color pieceColor = pieces_[move.from]->getColor();
    ScopedMove applied(*this, move);
    return !isCheck(pieceColor);
}

bool ChessBoard::isEnPassant(Move move) const {
    if (move.from < 0 || move.from >= NUM_SQUARES ||
        move.to < 0 || move.to >= NUM_SQUARES ||
        !pieces_[move.from] || pieces_[move.from]->getKind() != PieceKind::pawn ||
        pieces_[move.to] || history_.empty()) {
        return false;
    }

    const Piece& pawn = *pieces_[move.from];
    const int direction = pawn.getColor() == white ? -1 : 1;
    const int to_row = move.to / BOARD_SIZE;
    const int to_col = move.to % BOARD_SIZE;
    if (to_row != pawn.getRow() + direction ||
        std::abs(to_col - pawn.getCol()) != 1) {
        return false;
    }

    const int captured_index = Index(pawn.getRow(), to_col);
    const Piece* captured = pieces_[captured_index].get();
    const MoveRecord& last = history_.last();
    const int enemy_start_row = pawn.getColor() == white ? 1 : 6;
    return captured && captured->getKind() == PieceKind::pawn &&
           captured->getColor() != pawn.getColor() &&
           last.piece == PieceKind::pawn && last.color != pawn.getColor() &&
           last.move.from == Index(enemy_start_row, to_col) &&
           last.move.to == captured_index;
}

bool ChessBoard::isItPossibleToCastle(Color _color, int rook_index) const { // Проверка на рокировку для _color
    for(const auto& move_record : history_){
        if(move_record.color == _color){
            if(move_record.piece == PieceKind::king) return false;
            if(move_record.move.from == rook_index) return false;
        }
    }
    return true;
}

bool ChessBoard::isSquaredAttacked(int index, Color attackerColor) const {
    if (index < 0 || index >= NUM_SQUARES) return false;
    for(const auto& piece : pieces_){
        if(piece && piece->getColor() == attackerColor){
            if (piece->getKind() == PieceKind::pawn) {
                const int direction = attackerColor == white ? -1 : 1;
                if (index / BOARD_SIZE == piece->getRow() + direction &&
                    abs(index % BOARD_SIZE - piece->getCol()) == 1) {
                    return true;
                }
                continue;
            }
            if (piece->getKind() == PieceKind::king) {
                if (abs(index / BOARD_SIZE - piece->getRow()) <= 1 &&
                    abs(index % BOARD_SIZE - piece->getCol()) <= 1 &&
                    index != Index(piece->getRow(), piece->getCol())) {
                    return true;
                }
                continue;
            }
            if(piece->ISPOSSIBLEMOVE({Index(piece->getRow(), piece->getCol()), index}, *this)) {
                return true;
            }
        }
    }
    return false;
}

} // namespace ce
