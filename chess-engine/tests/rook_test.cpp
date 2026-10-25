#include <gtest/gtest.h>
#include "piece_test_support.hpp"

TEST(RookRules, MovementAndSpecialRules) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Rook>(board, 'd', 4, white);
    ASSERT_TRUE(board.pieceAt(square('d', 4))->FINDPOSSIBLEMOVES(board).size() == 14) << "Central rook must have fourteen moves on an empty board";
    ASSERT_TRUE(canMove(board, move('d', 4, 'd', 8))) << "Rook must move along a file";
    ASSERT_TRUE(canMove(board, move('d', 4, 'a', 4))) << "Rook must move along a rank";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'e', 5))) << "Rook cannot move diagonally";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'd', 4))) << "Rook cannot stay in place";

    put<Pawn>(board, 'd', 5, white);
    ASSERT_TRUE(!canMove(board, move('d', 4, 'd', 5))) << "Rook cannot capture its own piece";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'd', 6))) << "Rook cannot cross a piece";
    put<Pawn>(board, 'f', 4, black);
    ASSERT_TRUE(canMove(board, move('d', 4, 'f', 4))) << "Rook must capture an enemy piece";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'g', 4))) << "Rook cannot move beyond a capture";
}
