#pragma once

#include "types.hpp"
#include "move.hpp"
#include <vector>
#include <memory>
#include <string>

namespace ce {

class ChessBoard;

struct Piece {
    virtual ~Piece() = default;

    virtual int getRow() const = 0;
    virtual int getCol() const = 0;
    virtual int getPrice() const = 0;
    virtual Color getColor() const = 0;
    virtual PieceKind getKind() const = 0;
    virtual void setPosition(int row, int col) = 0;

	virtual std::string getSymbol() const = 0;
    virtual std::vector<Move> FINDPOSSIBLEMOVES(const ChessBoard& board) const = 0;
	virtual bool ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const = 0;
};

class PieceBase : public Piece {
protected:
    int row, col, price;
    Color color;
    const PieceKind kind;

    void addIfPossible(std::vector<Move>& moves, const ChessBoard& board,
                       int to_row, int to_col) const;
    void addRay(std::vector<Move>& moves, const ChessBoard& board,
                int row_step, int col_step) const;

public:
    PieceBase(int row, int col, int price, Color color, PieceKind kind);

    int getRow() const override;
    int getCol() const override;
    int getPrice() const override;
    Color getColor() const override;
    PieceKind getKind() const override;
    void setPosition(int row, int col) override;

};

std::unique_ptr<Piece> makePiece(PieceKind kind, int row, int col, int price, Color color);

struct Pawn : PieceBase {
	Pawn(int row, int col, int price, Color _color);
	std::string getSymbol() const override;
	std::vector<Move> FINDPOSSIBLEMOVES(const ChessBoard& board) const override;
	bool ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const override;
};

struct Knight : PieceBase {
	Knight(int row, int col, int price, Color _color);
	std::string getSymbol() const override;
	std::vector<Move> FINDPOSSIBLEMOVES(const ChessBoard& board) const override;
	bool ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const override;
};

struct Bishop : PieceBase {
	Bishop(int row, int col, int price, Color _color);
	std::string getSymbol() const override;
	std::vector<Move> FINDPOSSIBLEMOVES(const ChessBoard& board) const override;
	bool ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const override;
};

struct Rook  : PieceBase {
	Rook(int row, int col, int price, Color _color);
	std::string getSymbol() const override;
	std::vector<Move> FINDPOSSIBLEMOVES(const ChessBoard& board) const override;
	bool ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const override;
};

struct Queen  : PieceBase {
	Queen(int row, int col, int price, Color _color);
	std::string getSymbol() const override;
	std::vector<Move> FINDPOSSIBLEMOVES(const ChessBoard& board) const override;
	bool ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const override;
};

struct King : PieceBase {
	King(int row, int col, int price, Color _color);

	std::string getSymbol() const override;
	std::vector<Move> FINDPOSSIBLEMOVES(const ChessBoard& board) const override;
	bool ISPOSSIBLEMOVE(Move move, const ChessBoard& board) const override;
};

} // namespace ce
