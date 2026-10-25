#include <gtest/gtest.h>
#include "piece_test_support.hpp"

TEST(QueenRules, MovementAndSpecialRules) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Queen>(board, 'd', 4, white);
    ASSERT_TRUE(board.pieceAt(square('d', 4))->FINDPOSSIBLEMOVES(board).size() == 27) << "Central queen must have twenty-seven moves on an empty board";
    ASSERT_TRUE(canMove(board, move('d', 4, 'd', 8))) << "Queen must move along a file";
    ASSERT_TRUE(canMove(board, move('d', 4, 'a', 4))) << "Queen must move along a rank";
    ASSERT_TRUE(canMove(board, move('d', 4, 'h', 8))) << "Queen must move diagonally";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'e', 6))) << "Queen cannot move like a knight";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'd', 4))) << "Queen cannot stay in place";

    put<Pawn>(board, 'e', 5, white);
    ASSERT_TRUE(!canMove(board, move('d', 4, 'e', 5))) << "Queen cannot capture its own piece";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'f', 6))) << "Queen cannot cross a piece";
    put<Pawn>(board, 'd', 6, black);
    ASSERT_TRUE(canMove(board, move('d', 4, 'd', 6))) << "Queen must capture an enemy piece";
    ASSERT_TRUE(!canMove(board, move('d', 4, 'd', 7))) << "Queen cannot move beyond a capture";
}
