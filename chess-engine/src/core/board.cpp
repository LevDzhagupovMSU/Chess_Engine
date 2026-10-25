#include "board.hpp"
#include "piece.hpp"
#include "history.hpp"
#include "types.hpp"

#include <iostream>
#include <cstdlib>
#include <memory>
#include <string>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ce {

ChessBoard::ChessBoard() {
    pieces_.resize(NUM_SQUARES);

    pieces_[0] = std::make_unique<Rook>(0, 0, 500, black);
    pieces_[1] = std::make_unique<Knight>(0, 1, 320, black);
    pieces_[2] = std::make_unique<Bishop>(0, 2, 330, black);
    pieces_[3] = std::make_unique<Queen>(0, 3, 900, black);
    pieces_[4] = std::make_unique<King>(0, 4, 50000, black);
    pieces_[5] = std::make_unique<Bishop>(0, 5, 330, black);
    pieces_[6] = std::make_unique<Knight>(0, 6, 320, black);
    pieces_[7] = std::make_unique<Rook>(0, 7, 500, black);

    for(int col = 0; col < BOARD_SIZE; col++) {
        pieces_[BOARD_SIZE + col] = std::make_unique<Pawn>(1, col, 100, black);
    }

    for(int col = 0; col < BOARD_SIZE; col++) {
        pieces_[48 + col] = std::make_unique<Pawn>(6, col, 100, white);
    }

    pieces_[56] = std::make_unique<Rook>(7, 0, 500, white);
    pieces_[57] = std::make_unique<Knight>(7, 1, 320, white);
    pieces_[58] = std::make_unique<Bishop>(7, 2, 330, white);
    pieces_[59] = std::make_unique<Queen>(7, 3, 900, white);
    pieces_[60] = std::make_unique<King>(7, 4, 50000, white);
    pieces_[61] = std::make_unique<Bishop>(7, 5, 330, white);
    pieces_[62] = std::make_unique<Knight>(7, 6, 320, white);
    pieces_[63] = std::make_unique<Rook>(7, 7, 500, white);

    white_kingIndex_ = Index(7, 4);
    black_kingIndex_ = Index(0, 4);
}

ChessBoard::ChessBoard(const ChessBoard& board) {
    pieces_.resize(NUM_SQUARES);
    for(int k = 0; k < NUM_SQUARES; k++){
        if(board.pieces_[k]) {
            const Piece& piece = *board.pieces_[k];
            pieces_[k] = makePiece(
                piece.getKind(), piece.getRow(), piece.getCol(), piece.getPrice(), piece.getColor());
        }
    }
    history_ = board.history_;
    white_kingIndex_ = board.white_kingIndex_;
    black_kingIndex_ = board.black_kingIndex_;
}

ChessBoard& ChessBoard::operator=(ChessBoard other) noexcept {
    if(this == &other) return *this;

    std::swap(pieces_, other.pieces_);
    std::swap(history_, other.history_);
    std::swap(white_kingIndex_, other.white_kingIndex_);
    std::swap(black_kingIndex_, other.black_kingIndex_);

    return *this;
}

ChessBoard::ChessBoard(ChessBoard&& other) noexcept
    : white_kingIndex_(other.white_kingIndex_),
      black_kingIndex_(other.black_kingIndex_) {
    std::swap(pieces_, other.pieces_);
    std::swap(history_, other.history_);
}

ChessBoard& ChessBoard::operator=(ChessBoard&& other) noexcept {
    if(this == &other) return *this;

    std::swap(pieces_, other.pieces_);
    std::swap(history_, other.history_);
    std::swap(white_kingIndex_, other.white_kingIndex_);
    std::swap(black_kingIndex_, other.black_kingIndex_);

    return *this;
}

ChessBoard::~ChessBoard() = default;

void ChessBoard::clear() {
    for (auto& piece : pieces_) piece.reset();
    history_.clear();
    white_kingIndex_ = -1;
    black_kingIndex_ = -1;
}

void ChessBoard::setPiece(int row, int col, std::unique_ptr<Piece> piece) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        throw std::out_of_range("Invalid board square");
    }
    const int index = Index(row, col);
    if (piece && piece->getKind() == PieceKind::king) {
        const int existing = kingSquare(piece->getColor());
        if (existing != -1 && existing != index) {
            throw std::invalid_argument("King already exists on the board");
        }
    }
    if (pieces_[index] && pieces_[index]->getKind() == PieceKind::king) {
        (pieces_[index]->getColor() == white ? white_kingIndex_ : black_kingIndex_) = -1;
    }
    if (piece) {
        piece->setPosition(row, col);
        if (piece->getKind() == PieceKind::king) {
            (piece->getColor() == white ? white_kingIndex_ : black_kingIndex_) = index;
        }
    }
    pieces_[index] = std::move(piece);
}

const Piece* ChessBoard::pieceAt(int index) const noexcept {
    if (index < 0 || index >= NUM_SQUARES) return nullptr;
    return pieces_[index].get();
}

const History& ChessBoard::getHistory() const noexcept {
    return history_;
}

int ChessBoard::kingSquare(Color color) const noexcept {
    return color == white ? white_kingIndex_ : black_kingIndex_;
}

bool ChessBoard::makeMove(Move move) {
    if (!isLegalMove(move)) return false;
    applyMoveUnchecked(move);
    return true;
}

ChessBoard::ScopedMove::ScopedMove(ChessBoard& board, Move move)
    : board_(board), undo_(board.applyMoveUnchecked(move)) {}

ChessBoard::ScopedMove::~ScopedMove() {
    board_.undoMove(std::move(undo_));
}

ChessBoard::UndoState ChessBoard::applyMoveUnchecked(Move move) {
    auto& moving = pieces_[move.from];
    const PieceKind kind = moving->getKind();
    const Color color = moving->getColor();
    const bool en_passant = kind == PieceKind::pawn && isEnPassant(move);
    const bool captured = pieces_[move.to] != nullptr || en_passant;
    UndoState undo{move};
    undo.white_king_index = white_kingIndex_;
    undo.black_king_index = black_kingIndex_;
    undo.capture_square = en_passant
                              ? Index(move.from / BOARD_SIZE, move.to % BOARD_SIZE)
                              : move.to;

    std::unique_ptr<Piece> promotion_piece;
    if (kind == PieceKind::pawn &&
        move.to / BOARD_SIZE == (color == white ? 0 : 7)) {
        int price = 900;
        switch (move.promotion) {
        case PieceKind::knight: price = 320; break;
        case PieceKind::bishop: price = 330; break;
        case PieceKind::rook: price = 500; break;
        case PieceKind::queen: break;
        default: throw std::invalid_argument("Invalid promotion");
        }
        promotion_piece = makePiece(
            move.promotion, move.to / BOARD_SIZE, move.to % BOARD_SIZE, price, color);
    }

    history_.record(kind, move, color, captured);
    undo.captured = std::move(pieces_[undo.capture_square]);

    if (kind == PieceKind::king) {
        (color == white ? white_kingIndex_ : black_kingIndex_) = move.to;
        if (std::abs(move.to - move.from) == 2) {
            const bool kingside = move.to > move.from;
            undo.rook_from = Index(color == white ? 7 : 0, kingside ? 7 : 0);
            undo.rook_to = kingside ? move.to - 1 : move.to + 1;
            pieces_[undo.rook_to] = std::move(pieces_[undo.rook_from]);
            pieces_[undo.rook_to]->setPosition(
                undo.rook_to / BOARD_SIZE, undo.rook_to % BOARD_SIZE);
        }
    }

    pieces_[move.to] = std::move(moving);
    pieces_[move.to]->setPosition(move.to / BOARD_SIZE, move.to % BOARD_SIZE);

    if (promotion_piece) {
        undo.promoted_pawn = std::move(pieces_[move.to]);
        pieces_[move.to] = std::move(promotion_piece);
    }
    return undo;
}

void ChessBoard::undoMove(UndoState&& undo) {
    const Move move = undo.move;
    if (undo.promoted_pawn) {
        pieces_[move.to] = std::move(undo.promoted_pawn);
    }
    pieces_[move.from] = std::move(pieces_[move.to]);
    pieces_[move.from]->setPosition(move.from / BOARD_SIZE, move.from % BOARD_SIZE);

    if (undo.rook_from != -1) {
        pieces_[undo.rook_from] = std::move(pieces_[undo.rook_to]);
        pieces_[undo.rook_from]->setPosition(
            undo.rook_from / BOARD_SIZE, undo.rook_from % BOARD_SIZE);
    }

    pieces_[undo.capture_square] = std::move(undo.captured);
    white_kingIndex_ = undo.white_king_index;
    black_kingIndex_ = undo.black_king_index;
    history_.popLast();
}

int ChessBoard::Index(int row, int col) const {
    return BOARD_SIZE * row + col;
}

void ChessBoard::display() const {
    std::string razdel = "  +----+----+----+----+----+----+----+----+";
    std::cout << std::endl << razdel << std::endl;
    for(int row = 0; row < BOARD_SIZE; row++){
        std::cout << BOARD_SIZE - row << " | ";
        for(int col = 0; col < BOARD_SIZE; col++){
            if(pieces_[Index(row, col)] == nullptr){
                std::cout << "  " << " | ";
                continue;
            }
            std::cout << pieces_[Index(row, col)]->getSymbol() << " | ";
        }
        std::cout << std::endl << razdel << std::endl;
    }
    std::cout << "    a    b    c    d    e    f    g    h" << std::endl;
}

} // namespace ce
