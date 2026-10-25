#include "board.hpp"
#include "piece_test_support.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <vector>

static_assert(std::is_abstract_v<ce::Piece>);

TEST(BoardRules, InitialPositionHasTwentyMovesAndFourHundredReplies) {
    ce::ChessBoard board;

    int opening_moves = 0;
    int opening_replies = 0;
    for (int from = 0; from < ce::NUM_SQUARES; ++from) {
        const ce::Piece* piece = board.pieceAt(from);
        if (!piece || piece->getColor() != ce::white) continue;
        for (const ce::Move candidate : piece->FINDPOSSIBLEMOVES(board)) {
            if (!board.isLegalMove(candidate)) continue;
            ++opening_moves;
            ce::ChessBoard reply_board(board);
            ASSERT_TRUE(reply_board.makeMove(candidate));
            for (int square = 0; square < ce::NUM_SQUARES; ++square) {
                const ce::Piece* opponent = reply_board.pieceAt(square);
                if (!opponent || opponent->getColor() != ce::black) continue;
                for (const ce::Move reply : opponent->FINDPOSSIBLEMOVES(reply_board)) {
                    if (reply_board.isLegalMove(reply)) ++opening_replies;
                }
            }
        }
    }
    EXPECT_EQ(opening_moves, 20);
    EXPECT_EQ(opening_replies, 400);
}

TEST(BoardRules, CopyAndMoveHistoryAreIndependent) {
    ce::ChessBoard board;
    ce::ChessBoard copy(board);
    for (int index = 0; index < ce::NUM_SQUARES; ++index) {
        ASSERT_EQ(board.pieceAt(index) != nullptr,
                  copy.pieceAt(index) != nullptr);
        if (board.pieceAt(index) &&
            (board.pieceAt(index) == copy.pieceAt(index) ||
             board.pieceAt(index)->getKind() != copy.pieceAt(index)->getKind())) {
            FAIL() << "Piece copy has incorrect identity or kind";
        }
    }

    const ce::Move move{52, 36}; // e2-e4
    ASSERT_TRUE(copy.pieceAt(move.from)->ISPOSSIBLEMOVE(move, copy));
    EXPECT_FALSE(copy.pieceAt(move.from)->ISPOSSIBLEMOVE({move.from - 1, move.to}, copy));
    EXPECT_FALSE(copy.pieceAt(move.from)->ISPOSSIBLEMOVE({move.from, ce::NUM_SQUARES}, copy));
    ASSERT_TRUE(copy.makeMove(move));

    ASSERT_NE(board.pieceAt(move.from), nullptr);
    EXPECT_EQ(board.pieceAt(move.to), nullptr);
    EXPECT_EQ(copy.pieceAt(move.from), nullptr);
    ASSERT_NE(copy.pieceAt(move.to), nullptr);
    EXPECT_EQ(copy.pieceAt(move.to)->getRow(), 4);
    EXPECT_EQ(copy.pieceAt(move.to)->getCol(), 4);
    ASSERT_EQ(copy.getHistory().size(), 1u);
    EXPECT_EQ(copy.getHistory().at(0).piece, ce::PieceKind::pawn);
    EXPECT_EQ(copy.getHistory().at(0).move.from, move.from);
    EXPECT_EQ(copy.getHistory().at(0).move.to, move.to);

    ce::ChessBoard replay(copy);
    ASSERT_EQ(replay.getHistory().size(), 1u);
    ASSERT_NE(replay.pieceAt(move.to), nullptr);
    EXPECT_NE(replay.pieceAt(move.to), copy.pieceAt(move.to));
}

TEST(BoardRules, PlacementMaintainsKingPositionAndCoordinates) {
    ce::ChessBoard board;
    board.clear();
    EXPECT_EQ(board.kingSquare(ce::white), -1);
    EXPECT_EQ(board.pieceAt(-1), nullptr);
    EXPECT_EQ(board.pieceAt(ce::NUM_SQUARES), nullptr);

    board.setPiece(4, 4, std::make_unique<ce::King>(0, 0, 50000, ce::white));
    ASSERT_NE(board.pieceAt(36), nullptr);
    EXPECT_EQ(board.pieceAt(36)->getRow(), 4);
    EXPECT_EQ(board.pieceAt(36)->getCol(), 4);
    EXPECT_EQ(board.kingSquare(ce::white), 36);
    EXPECT_THROW(board.setPiece(5, 5, std::make_unique<ce::King>(5, 5, 50000, ce::white)),
                 std::invalid_argument);
    EXPECT_EQ(board.kingSquare(ce::white), 36);

    board.setPiece(4, 4, nullptr);
    EXPECT_EQ(board.pieceAt(36), nullptr);
    EXPECT_EQ(board.kingSquare(ce::white), -1);
}

TEST(BoardRules, RejectedMoveDoesNotChangePosition) {
    ce::ChessBoard board;
    EXPECT_FALSE(board.makeMove({52, 28})); // e2-e5
    ASSERT_NE(board.pieceAt(52), nullptr);
    EXPECT_EQ(board.pieceAt(28), nullptr);
    EXPECT_TRUE(board.getHistory().empty());

    ASSERT_TRUE(board.makeMove({52, 36})); // e2-e4
    EXPECT_EQ(board.pieceAt(52), nullptr);
    ASSERT_NE(board.pieceAt(36), nullptr);
    EXPECT_EQ(board.getHistory().size(), 1u);
}

TEST(BoardRules, KingCannotBeCaptured) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'e', 1, ce::white);
    test::put<ce::King>(board, 'e', 8, ce::black);
    test::put<ce::Rook>(board, 'e', 7, ce::white);

    const ce::Move capture = test::move('e', 7, 'e', 8);
    EXPECT_FALSE(board.isLegalMove(capture));
    EXPECT_FALSE(board.makeMove(capture));
    EXPECT_EQ(board.kingSquare(ce::black), test::square('e', 8));
    ASSERT_NE(board.pieceAt(test::square('e', 8)), nullptr);
}

TEST(MoveGeneration, MatchesPieceRulesInRepresentativePositions) {
    using MoveKey = std::tuple<int, int, int>;
    const auto key = [](ce::Move move) {
        return MoveKey{move.from, move.to, static_cast<int>(move.promotion)};
    };
    const auto verify = [&](const ce::ChessBoard& board) {
        for (int from = 0; from < ce::NUM_SQUARES; ++from) {
            const ce::Piece* piece = board.pieceAt(from);
            if (!piece) continue;

            std::vector<MoveKey> expected;
            std::vector<MoveKey> generated;
            for (int to = 0; to < ce::NUM_SQUARES; ++to) {
                for (const ce::PieceKind promotion : {
                         ce::PieceKind::queen, ce::PieceKind::rook,
                         ce::PieceKind::bishop, ce::PieceKind::knight}) {
                    if (promotion != ce::PieceKind::queen &&
                        (piece->getKind() != ce::PieceKind::pawn ||
                         to / ce::BOARD_SIZE != (piece->getColor() == ce::white ? 0 : 7))) {
                        continue;
                    }
                    const ce::Move candidate{from, to, promotion};
                    if (piece->ISPOSSIBLEMOVE(candidate, board)) {
                        expected.push_back(key(candidate));
                    }
                }
            }
            for (const ce::Move move : piece->FINDPOSSIBLEMOVES(board)) {
                generated.push_back(key(move));
            }
            std::sort(expected.begin(), expected.end());
            std::sort(generated.begin(), generated.end());
            EXPECT_EQ(generated, expected) << "Piece at square " << from;
        }
    };

    ce::ChessBoard initial;
    verify(initial);

    auto tactical = test::emptyBoard();
    test::put<ce::King>(tactical, 'e', 1, ce::white);
    test::put<ce::King>(tactical, 'e', 8, ce::black);
    test::put<ce::Queen>(tactical, 'd', 4, ce::white);
    test::put<ce::Rook>(tactical, 'a', 4, ce::white);
    test::put<ce::Bishop>(tactical, 'g', 2, ce::white);
    test::put<ce::Knight>(tactical, 'b', 5, ce::white);
    test::put<ce::Pawn>(tactical, 'e', 5, ce::white);
    test::put<ce::Pawn>(tactical, 'd', 7, ce::black);
    test::put<ce::Rook>(tactical, 'h', 4, ce::black);
    test::play(tactical, test::move('d', 7, 'd', 5));
    verify(tactical);

    auto promotion = test::emptyBoard();
    test::put<ce::Pawn>(promotion, 'a', 7, ce::white);
    test::put<ce::Rook>(promotion, 'b', 8, ce::black);
    verify(promotion);
}
