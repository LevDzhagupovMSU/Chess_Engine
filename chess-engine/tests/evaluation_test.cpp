#include "ai.hpp"
#include "piece_test_support.hpp"

#include <gtest/gtest.h>
#include <memory>

TEST(EvaluationRules, ScoresAreOppositeForTheTwoSides) {
    ce::Ai ai;
    ce::ChessBoard board;
    EXPECT_EQ(ai.evaluatePosition(board, ce::white), 0);
    EXPECT_EQ(ai.evaluatePosition(board, ce::white),
              -ai.evaluatePosition(board, ce::black));

    ASSERT_TRUE(board.makeMove(test::move('e', 2, 'e', 4)));
    EXPECT_EQ(ai.evaluatePosition(board, ce::white),
              -ai.evaluatePosition(board, ce::black));
}

TEST(EvaluationRules, ProtectedPieceIsWorthMoreThanHangingPiece) {
    auto defended = test::emptyBoard();
    defended.setPiece(7, 7, std::make_unique<ce::King>(7, 7, 50000, ce::white));
    defended.setPiece(0, 7, std::make_unique<ce::King>(0, 7, 50000, ce::black));
    defended.setPiece(4, 3, std::make_unique<ce::Rook>(4, 3, 500, ce::white));
    defended.setPiece(0, 3, std::make_unique<ce::Rook>(0, 3, 500, ce::black));
    defended.setPiece(7, 6, std::make_unique<ce::Bishop>(7, 6, 330, ce::white));

    ce::ChessBoard hanging(defended);
    hanging.setPiece(7, 6, nullptr);
    hanging.setPiece(6, 7, std::make_unique<ce::Bishop>(6, 7, 330, ce::white));

    ce::Ai ai;
    EXPECT_GT(ai.evaluatePosition(defended, ce::white),
              ai.evaluatePosition(hanging, ce::white) + 100);
}

TEST(EvaluationRules, PieceSquareTableDoesNotCountQueenMaterialTwice) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    ce::Ai ai;
    const int without_queen = ai.evaluatePosition(board, ce::white);

    board.setPiece(5, 3, std::make_unique<ce::Queen>(5, 3, 900, ce::white));
    const int queen_contribution = ai.evaluatePosition(board, ce::white) - without_queen;
    EXPECT_GT(queen_contribution, 800);
    EXPECT_LT(queen_contribution, 1400);
}
