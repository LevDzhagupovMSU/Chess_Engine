#include <gtest/gtest.h>
#include "piece_test_support.hpp"

TEST(KingRules, BasicMovementAndCheck) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 4, white);
    ASSERT_TRUE(board.pieceAt(square('e', 4))->FINDPOSSIBLEMOVES(board).size() == 8) << "Central king must have eight adjacent moves";
    ASSERT_TRUE(canMove(board, move('e', 4, 'f', 5))) << "King must move one square diagonally";
    ASSERT_TRUE(!canMove(board, move('e', 4, 'e', 6))) << "King cannot move two squares normally";
    put<Pawn>(board, 'd', 5, white);
    ASSERT_TRUE(!canMove(board, move('e', 4, 'd', 5))) << "King cannot capture its own piece";
    put<Pawn>(board, 'd', 5, black);
    ASSERT_TRUE(canMove(board, move('e', 4, 'd', 5))) << "King must capture an enemy piece";
    put<Rook>(board, 'f', 8, black);
    ASSERT_TRUE(canMove(board, move('e', 4, 'f', 4))) << "King has a pseudo-legal move into attack";
    ASSERT_TRUE(!board.isLegalMove(move('e', 4, 'f', 4))) << "King may not move into check";
}

TEST(KingRules, WhiteKingsideCastling) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 1, white);
    put<Rook>(board, 'a', 1, white);
    put<Rook>(board, 'h', 1, white);
    put<King>(board, 'e', 8, black);
    ASSERT_TRUE(canMove(board, move('e', 1, 'g', 1))) << "White kingside castling must be available";
    ASSERT_TRUE(canMove(board, move('e', 1, 'c', 1))) << "White queenside castling must be available";
    play(board, move('e', 1, 'g', 1));
    ASSERT_TRUE(board.pieceAt(square('g', 1))->getKind() == PieceKind::king &&
            board.pieceAt(square('f', 1))->getKind() == PieceKind::rook &&
            !board.pieceAt(square('h', 1)) &&
            board.kingSquare(white) == square('g', 1)) << "Castling must move both king and rook";
}

TEST(KingRules, BlackKingsideCastling) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 8, black);
    put<Rook>(board, 'h', 8, black);
    put<King>(board, 'e', 1, white);
    ASSERT_TRUE(canMove(board, move('e', 8, 'g', 8))) << "Black kingside castling must be available";
    play(board, move('e', 8, 'g', 8));
    ASSERT_TRUE(board.pieceAt(square('g', 8))->getKind() == PieceKind::king &&
            board.pieceAt(square('f', 8))->getKind() == PieceKind::rook &&
            board.kingSquare(black) == square('g', 8)) << "Black castling must move king and rook";
}

TEST(KingRules, WhiteQueensideCastling) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 1, white);
    put<Rook>(board, 'a', 1, white);
    put<King>(board, 'e', 8, black);
    play(board, move('e', 1, 'c', 1));
    ASSERT_TRUE(board.pieceAt(square('c', 1))->getKind() == PieceKind::king &&
            board.pieceAt(square('d', 1))->getKind() == PieceKind::rook) << "Queenside castling must move rook to d-file";
}

TEST(KingRules, CastlingRequiresRookAndClearPath) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 1, white);
    put<King>(board, 'e', 8, black);
    ASSERT_TRUE(!canMove(board, move('e', 1, 'g', 1))) << "Castling requires a rook";
    put<Knight>(board, 'h', 1, white);
    ASSERT_TRUE(!canMove(board, move('e', 1, 'g', 1))) << "Castling requires the correct rook";
    put<Rook>(board, 'h', 1, white);
    put<Knight>(board, 'f', 1, white);
    ASSERT_TRUE(!canMove(board, move('e', 1, 'g', 1))) << "Castling path must be empty";
}

TEST(KingRules, CannotCastleThroughPawnAttack) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 1, white);
    put<Rook>(board, 'h', 1, white);
    put<King>(board, 'e', 8, black);
    put<Pawn>(board, 'g', 2, black);
    ASSERT_TRUE(board.isSquaredAttacked(square('f', 1), black)) << "Pawn must attack an empty diagonal square";
    ASSERT_TRUE(!canMove(board, move('e', 1, 'g', 1))) << "King cannot castle through a pawn attack";
}

TEST(KingRules, CannotCastleOntoAttackedSquare) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 1, white);
    put<Rook>(board, 'h', 1, white);
    put<King>(board, 'a', 8, black);
    put<Rook>(board, 'g', 8, black);
    ASSERT_TRUE(!canMove(board, move('e', 1, 'g', 1))) << "King cannot castle onto an attacked square";
}

TEST(KingRules, CannotCastleOutOfCheck) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 1, white);
    put<Rook>(board, 'h', 1, white);
    put<King>(board, 'a', 8, black);
    put<Rook>(board, 'e', 8, black);
    ASSERT_TRUE(board.isCheck(white)) << "Rook must give check along an open file";
    ASSERT_TRUE(!canMove(board, move('e', 1, 'g', 1))) << "King cannot castle out of check";
}

TEST(KingRules, RookMoveRevokesCastling) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 1, white);
    put<Rook>(board, 'h', 1, white);
    put<King>(board, 'e', 8, black);
    play(board, move('h', 1, 'h', 2));
    play(board, move('h', 2, 'h', 1));
    ASSERT_TRUE(!canMove(board, move('e', 1, 'g', 1))) << "Rook movement must revoke castling rights";
}

TEST(KingRules, KingMoveRevokesCastling) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'e', 1, white);
    put<Rook>(board, 'h', 1, white);
    put<King>(board, 'e', 8, black);
    play(board, move('e', 1, 'e', 2));
    play(board, move('e', 2, 'e', 1));
    ASSERT_TRUE(!canMove(board, move('e', 1, 'g', 1))) << "King movement must revoke castling rights";
}
