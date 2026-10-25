#include "game.hpp"
#include "piece_test_support.hpp"

#include <gtest/gtest.h>
#include <utility>

TEST(GameRules, AlternatesTurnsAndRejectsInvalidMovesWithoutChangingState) {
    ce::Game game;
    EXPECT_EQ(game.sideToMove(), ce::white);
    EXPECT_EQ(game.status(), ce::GameStatus::ongoing);

    EXPECT_FALSE(game.makeMove(test::move('e', 7, 'e', 5)));
    EXPECT_FALSE(game.makeMove({-1, 0}));
    EXPECT_EQ(game.sideToMove(), ce::white);
    EXPECT_TRUE(game.board().getHistory().empty());

    ASSERT_TRUE(game.makeMove(test::move('e', 2, 'e', 4)));
    EXPECT_EQ(game.sideToMove(), ce::black);
    EXPECT_FALSE(game.makeMove(test::move('d', 2, 'd', 4)));
    EXPECT_EQ(game.board().getHistory().size(), 1u);

    ASSERT_TRUE(game.makeMove(test::move('e', 7, 'e', 5)));
    EXPECT_EQ(game.sideToMove(), ce::white);
    EXPECT_EQ(game.board().getHistory().size(), 2u);
}

TEST(GameRules, AiMoveUsesTheCurrentSide) {
    ce::Game game;
    const ce::Move white_move = game.getAiMove(1);
    ASSERT_NE(game.board().pieceAt(white_move.from), nullptr);
    EXPECT_EQ(game.board().pieceAt(white_move.from)->getColor(), ce::white);
    ce::Game white_verification(game);
    EXPECT_TRUE(white_verification.makeMove(white_move));

    ASSERT_TRUE(game.makeMove(test::move('e', 2, 'e', 4)));
    const ce::Move black_move = game.getAiMove(1);
    ASSERT_NE(game.board().pieceAt(black_move.from), nullptr);
    EXPECT_EQ(game.board().pieceAt(black_move.from)->getColor(), ce::black);
    ce::Game black_verification(game);
    EXPECT_TRUE(black_verification.makeMove(black_move));
}

TEST(GameRules, AiAccountsForTheBestOpponentReply) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    test::put<ce::Queen>(board, 'd', 4, ce::white);
    test::put<ce::Pawn>(board, 'd', 5, ce::black);
    test::put<ce::Rook>(board, 'd', 8, ce::black);

    ce::Game game(std::move(board));
    const ce::Move chosen = game.getAiMove(2);
    ce::Game verification(game);
    EXPECT_TRUE(verification.makeMove(chosen));
    EXPECT_FALSE(chosen.from == test::square('d', 4) &&
                 chosen.to == test::square('d', 5))
        << "Qxd5 loses the queen to Rxd5";
}

TEST(GameRules, AiRecognizesMateAndStalemateAtSearchHorizon) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 8, ce::black);
    test::put<ce::King>(board, 'c', 6, ce::white);
    test::put<ce::Queen>(board, 'c', 7, ce::white);

    ce::Game game(std::move(board));
    ce::Game stalemate(game.board());
    ASSERT_TRUE(stalemate.makeMove(test::move('c', 7, 'b', 6)));
    EXPECT_EQ(stalemate.status(), ce::GameStatus::stalemate);
    EXPECT_EQ(stalemate.getAiMove(1).from, -1);

    const ce::Move chosen = game.getAiMove(1);
    ASSERT_TRUE(game.makeMove(chosen));
    EXPECT_EQ(game.status(), ce::GameStatus::checkmate)
        << "A mate in one must outrank a stalemate in one";
    EXPECT_EQ(game.getAiMove(1).from, -1);
}

TEST(GameRules, DetectsCheckAndCheckmateAfterMoves) {
    ce::Game game;
    ASSERT_TRUE(game.makeMove(test::move('f', 2, 'f', 3)));
    ASSERT_TRUE(game.makeMove(test::move('e', 7, 'e', 5)));
    ASSERT_TRUE(game.makeMove(test::move('g', 2, 'g', 4)));
    ASSERT_TRUE(game.makeMove(test::move('d', 8, 'h', 4)));

    EXPECT_EQ(game.sideToMove(), ce::white);
    EXPECT_EQ(game.status(), ce::GameStatus::checkmate);
    EXPECT_FALSE(game.makeMove(test::move('e', 2, 'e', 4)));
    EXPECT_EQ(game.board().getHistory().size(), 4u);
}

TEST(GameRules, DetectsCheckInSuppliedPosition) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    test::put<ce::Rook>(board, 'a', 8, ce::black);

    ce::Game game(std::move(board), ce::white);
    EXPECT_EQ(game.status(), ce::GameStatus::check);
    EXPECT_FALSE(game.makeMove(test::move('a', 1, 'a', 2)));
    EXPECT_TRUE(game.makeMove(test::move('a', 1, 'b', 1)));
    EXPECT_EQ(game.status(), ce::GameStatus::ongoing);
}

TEST(GameRules, DetectsStalemateAndRefusesFurtherMoves) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 8, ce::black);
    test::put<ce::King>(board, 'c', 6, ce::white);
    test::put<ce::Queen>(board, 'b', 6, ce::white);

    ce::Game game(std::move(board), ce::black);
    EXPECT_EQ(game.status(), ce::GameStatus::stalemate);
    EXPECT_FALSE(game.makeMove(test::move('a', 8, 'a', 7)));
}

TEST(GameRules, RequiresBothKingsInSuppliedPosition) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'e', 1, ce::white);
    EXPECT_THROW(ce::Game(std::move(board)), std::invalid_argument);
}

TEST(GameRules, FiftyMoveDrawRequiresAClaimAfterOneHundredHalfmoves) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    ce::Game game(std::move(board));

    EXPECT_FALSE(game.canClaimFiftyMoveDraw());
    EXPECT_FALSE(game.claimFiftyMoveDraw());
    for (int cycle = 0; cycle < 25; ++cycle) {
        ASSERT_TRUE(game.makeMove(test::move('a', 1, 'b', 1)));
        ASSERT_TRUE(game.makeMove(test::move('h', 8, 'g', 8)));
        ASSERT_TRUE(game.makeMove(test::move('b', 1, 'a', 1)));
        EXPECT_EQ(game.halfmoveClock(), static_cast<std::size_t>(cycle * 4 + 3));
        EXPECT_FALSE(game.canClaimFiftyMoveDraw());
        ASSERT_TRUE(game.makeMove(test::move('g', 8, 'h', 8)));
    }

    EXPECT_EQ(game.halfmoveClock(), 100u);
    EXPECT_EQ(game.status(), ce::GameStatus::ongoing);
    ASSERT_TRUE(game.canClaimFiftyMoveDraw());
    ASSERT_TRUE(game.claimFiftyMoveDraw());
    EXPECT_EQ(game.status(), ce::GameStatus::fifty_move_draw);
    EXPECT_FALSE(game.makeMove(test::move('a', 1, 'b', 1)));
    EXPECT_EQ(game.getAiMove(1).from, -1);
    EXPECT_FALSE(game.claimFiftyMoveDraw());
}

TEST(GameRules, PawnMovesAndCapturesResetHalfmoveClock) {
    ce::Game game;
    EXPECT_FALSE(game.makeMove(test::move('e', 2, 'e', 5)));
    EXPECT_EQ(game.halfmoveClock(), 0u);

    ASSERT_TRUE(game.makeMove(test::move('g', 1, 'f', 3)));
    ASSERT_TRUE(game.makeMove(test::move('g', 8, 'f', 6)));
    EXPECT_EQ(game.halfmoveClock(), 2u);
    ASSERT_TRUE(game.makeMove(test::move('e', 2, 'e', 4)));
    EXPECT_EQ(game.halfmoveClock(), 0u);
    ASSERT_TRUE(game.makeMove(test::move('d', 7, 'd', 5)));
    EXPECT_EQ(game.halfmoveClock(), 0u);
    ASSERT_TRUE(game.makeMove(test::move('e', 4, 'd', 5)));
    EXPECT_EQ(game.halfmoveClock(), 0u);

    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'h', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    test::put<ce::Rook>(board, 'a', 2, ce::white);
    test::put<ce::Knight>(board, 'a', 7, ce::black);
    ce::Game capture_game(std::move(board));
    ASSERT_TRUE(capture_game.makeMove(test::move('a', 2, 'a', 3)));
    ASSERT_TRUE(capture_game.makeMove(test::move('h', 8, 'g', 8)));
    EXPECT_EQ(capture_game.halfmoveClock(), 2u);
    ce::Game resumed(capture_game.board(), ce::white);
    EXPECT_EQ(resumed.halfmoveClock(), 2u);
    ASSERT_TRUE(capture_game.makeMove(test::move('a', 3, 'a', 7)));
    EXPECT_EQ(capture_game.halfmoveClock(), 0u);
    EXPECT_TRUE(capture_game.board().getHistory().last().captured);
    ce::Game after_capture(capture_game.board(), ce::black);
    EXPECT_EQ(after_capture.halfmoveClock(), 0u);
}

TEST(GameRules, CanClaimBeforeTheIntendedHundredthHalfmove) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    ce::Game game(std::move(board));

    for (int cycle = 0; cycle < 24; ++cycle) {
        ASSERT_TRUE(game.makeMove(test::move('a', 1, 'b', 1)));
        ASSERT_TRUE(game.makeMove(test::move('h', 8, 'g', 8)));
        ASSERT_TRUE(game.makeMove(test::move('b', 1, 'a', 1)));
        ASSERT_TRUE(game.makeMove(test::move('g', 8, 'h', 8)));
    }
    ASSERT_TRUE(game.makeMove(test::move('a', 1, 'b', 1)));
    ASSERT_TRUE(game.makeMove(test::move('h', 8, 'g', 8)));
    ASSERT_TRUE(game.makeMove(test::move('b', 1, 'a', 1)));
    ASSERT_EQ(game.halfmoveClock(), 99u);
    EXPECT_FALSE(game.canClaimFiftyMoveDraw());
    EXPECT_FALSE(game.canClaimFiftyMoveDraw(test::move('g', 8, 'e', 8)));
    ASSERT_TRUE(game.canClaimFiftyMoveDraw(test::move('g', 8, 'h', 8)));
    ASSERT_TRUE(game.claimFiftyMoveDraw(test::move('g', 8, 'h', 8)));
    EXPECT_EQ(game.status(), ce::GameStatus::fifty_move_draw);
    EXPECT_EQ(game.board().getHistory().size(), 99u);
}

TEST(BoardRules, LegalMoveSearchRejectsKingCapture) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'e', 8, ce::black);
    test::put<ce::King>(board, 'e', 7, ce::white);

    EXPECT_FALSE(board.isLegalMove(test::move('e', 8, 'e', 7)));
    EXPECT_FALSE(board.hasAnyLegalMove(ce::black));
}
