#include <gtest/gtest.h>
#include "piece_test_support.hpp"

TEST(BishopRules, MovementAndSpecialRules) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Bishop>(board, 'd', 4, white);
    ASSERT_TRUE(board.pieceAt(square('d', 4))->FINDPOSSIBLEMOVES(board).size() == 13) << "Central bishop must have thirteen moves on an empty board";
    ASSERT_TRUE(canMove(board, move('d', 4, 'a', 1))) << "Bishop must move diagonally";
    ASSERT_TRUE(canMove(board, move('d', 4, 'h', 8))) << "Bishop must move diagonally";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'd', 6))) << "Bishop cannot move straight";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'e', 6))) << "Bishop cannot move like a knight";

    put<Pawn>(board, 'e', 5, white);
    ASSERT_TRUE(!canMove(board, move('d', 4, 'e', 5))) << "Bishop cannot capture its own piece";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'f', 6))) << "Bishop cannot cross a piece";
    put<Pawn>(board, 'e', 5, black);
    ASSERT_TRUE(canMove(board, move('d', 4, 'e', 5))) << "Bishop must capture an enemy piece";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'f', 6))) << "Bishop cannot pass a captured piece";
}
