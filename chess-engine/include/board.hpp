#pragma once
#include "types.hpp"
#include "piece.hpp"
#include "history.hpp"
#include "move.hpp"
#include <memory>
#include <vector>

namespace ce {

class Ai;

class ChessBoard {
    friend class Ai;

    struct UndoState {
        Move move;
        std::unique_ptr<Piece> captured{};
        std::unique_ptr<Piece> promoted_pawn{};
        int capture_square = -1;
        int rook_from = -1;
        int rook_to = -1;
        int white_king_index = -1;
        int black_king_index = -1;
    };

    class ScopedMove {
        ChessBoard& board_;
        UndoState undo_;
    public:
        ScopedMove(ChessBoard& board, Move move);
        ~ScopedMove();
        ScopedMove(const ScopedMove&) = delete;
        ScopedMove& operator=(const ScopedMove&) = delete;
    };

    std::vector<std::unique_ptr<Piece>> pieces_;
    History history_;
    int white_kingIndex_ = -1;
    int black_kingIndex_ = -1;

    UndoState applyMoveUnchecked(Move move);
    void undoMove(UndoState&& undo);
public:
	ChessBoard();
	ChessBoard(const ChessBoard& board);
    ChessBoard& operator=(ChessBoard other) noexcept;

    ChessBoard(ChessBoard&& other) noexcept;
    ChessBoard& operator=(ChessBoard&& other) noexcept;

    ~ChessBoard();

    void clear();
    void setPiece(int row, int col, std::unique_ptr<Piece> piece);
    const Piece* pieceAt(int index) const noexcept;
    const History& getHistory() const noexcept;
    int kingSquare(Color color) const noexcept;
    int Index(int row, int col) const;
    void display() const;

    bool makeMove(Move move);
    bool isEnPassant(Move move) const;
    bool isCheck(Color _color) const;
    bool isCheckMate(Color _color);
    bool hasAnyLegalMove(Color _color);
    bool wouldSafeKing(Move move);
    bool isLegalMove(Move move);
	bool isItPossibleToCastle(Color _color, int rook_index) const;
	bool isSquaredAttacked(int index, Color attackerColor) const;

};

} // namespace ce
