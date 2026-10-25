#include <gtest/gtest.h>
#include "piece_test_support.hpp"

TEST(KnightRules, MovementAndSpecialRules) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Knight>(board, 'd', 4, white);
    ASSERT_TRUE(board.pieceAt(square('d', 4))->FINDPOSSIBLEMOVES(board).size() == 8) << "Central knight must have eight moves on an empty board";
    ASSERT_TRUE(canMove(board, move('d', 4, 'b', 3))) << "Knight must move in an L shape";
    ASSERT_TRUE(canMove(board, move('d', 4, 'e', 6))) << "Knight must move in an L shape";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'd', 5))) << "Knight cannot move straight";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'e', 5))) << "Knight cannot move diagonally";
    ASSERT_TRUE(!board.isLegalMove(move('d', 4, 'e', 5))) << "Board must reject a move that breaks the piece's movement rule";

    put<Pawn>(board, 'd', 5, white);
    ASSERT_TRUE(canMove(board, move('d', 4, 'e', 6))) << "Knight must jump over pieces";
    put<Pawn>(board, 'e', 6, white);
    ASSERT_TRUE(!canMove(board, move('d', 4, 'e', 6))) << "Knight cannot capture its own piece";
    put<Pawn>(board, 'e', 6, black);
    ASSERT_TRUE(canMove(board, move('d', 4, 'e', 6))) << "Knight must capture an enemy piece";
}
