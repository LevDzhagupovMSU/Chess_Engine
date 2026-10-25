#include "game.hpp"
#include "piece_test_support.hpp"

#include <gtest/gtest.h>
#include <array>
#include <utility>
#include <vector>

namespace {

struct BoardSnapshot {
    std::array<const ce::Piece*, ce::NUM_SQUARES> pieces{};
    std::array<int, ce::NUM_SQUARES> rows{};
    std::array<int, ce::NUM_SQUARES> cols{};
    int white_king = -1;
    int black_king = -1;
    std::vector<ce::MoveRecord> history;

    explicit BoardSnapshot(const ce::ChessBoard& board)
        : white_king(board.kingSquare(ce::white)),
          black_king(board.kingSquare(ce::black)),
          history(board.getHistory().begin(), board.getHistory().end()) {
        for (int i = 0; i < ce::NUM_SQUARES; ++i) {
            pieces[i] = board.pieceAt(i);
            if (pieces[i]) {
                rows[i] = pieces[i]->getRow();
                cols[i] = pieces[i]->getCol();
            }
        }
    }

    void expectRestored(const ce::ChessBoard& board) const {
        EXPECT_EQ(board.kingSquare(ce::white), white_king);
        EXPECT_EQ(board.kingSquare(ce::black), black_king);
        ASSERT_EQ(board.getHistory().size(), history.size());
        for (std::size_t i = 0; i < history.size(); ++i) {
            const auto& actual = board.getHistory().at(i);
            EXPECT_EQ(actual.piece, history[i].piece);
            EXPECT_EQ(actual.move.from, history[i].move.from);
            EXPECT_EQ(actual.move.to, history[i].move.to);
            EXPECT_EQ(actual.move.promotion, history[i].move.promotion);
            EXPECT_EQ(actual.color, history[i].color);
            EXPECT_EQ(actual.captured, history[i].captured);
        }
        for (int i = 0; i < ce::NUM_SQUARES; ++i) {
            EXPECT_EQ(board.pieceAt(i), pieces[i]) << "Square " << i;
            if (pieces[i]) {
                EXPECT_EQ(pieces[i]->getRow(), rows[i]);
                EXPECT_EQ(pieces[i]->getCol(), cols[i]);
            }
        }
    }
};

} // namespace

TEST(BoardUndo, RestoresCapturedPieceAndHistory) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    test::put<ce::Rook>(board, 'd', 4, ce::white);
    test::put<ce::Knight>(board, 'd', 7, ce::black);
    const BoardSnapshot before(board);

    ASSERT_TRUE(board.isLegalMove(test::move('d', 4, 'd', 7)));
    before.expectRestored(board);
    ASSERT_TRUE(board.makeMove(test::move('d', 4, 'd', 7)));
    EXPECT_TRUE(board.getHistory().last().captured);
    EXPECT_EQ(board.pieceAt(test::square('d', 7))->getKind(), ce::PieceKind::rook);
}

TEST(BoardUndo, RestoresEnPassantCapture) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    test::put<ce::Pawn>(board, 'e', 5, ce::white);
    test::put<ce::Pawn>(board, 'd', 7, ce::black);
    ASSERT_TRUE(board.makeMove(test::move('d', 7, 'd', 5)));
    const BoardSnapshot before(board);

    ASSERT_TRUE(board.isLegalMove(test::move('e', 5, 'd', 6)));
    before.expectRestored(board);
    EXPECT_TRUE(board.isEnPassant(test::move('e', 5, 'd', 6)));
    ASSERT_TRUE(board.makeMove(test::move('e', 5, 'd', 6)));
    EXPECT_TRUE(board.getHistory().last().captured);
}

TEST(BoardUndo, RestoresCastlingOnBothSides) {
    for (const ce::Color color : {ce::white, ce::black}) {
        for (const char rook_file : {'a', 'h'}) {
            auto board = test::emptyBoard();
            const int rank = color == ce::white ? 1 : 8;
            test::put<ce::King>(board, 'e', rank, color);
            test::put<ce::Rook>(board, rook_file, rank, color);
            test::put<ce::King>(board, 'e', color == ce::white ? 8 : 1,
                                color == ce::white ? ce::black : ce::white);
            const char destination = rook_file == 'a' ? 'c' : 'g';
            const ce::Move castle = test::move('e', rank, destination, rank);
            const BoardSnapshot before(board);

            ASSERT_TRUE(board.isLegalMove(castle));
            before.expectRestored(board);
            ASSERT_TRUE(board.makeMove(castle));
            EXPECT_EQ(board.kingSquare(color), castle.to);
        }
    }
}

TEST(BoardUndo, RestoresPawnAfterCapturePromotion) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    test::put<ce::Pawn>(board, 'a', 7, ce::white);
    test::put<ce::Rook>(board, 'b', 8, ce::black);
    ce::Move promotion = test::move('a', 7, 'b', 8);
    promotion.promotion = ce::PieceKind::knight;
    const BoardSnapshot before(board);

    ASSERT_TRUE(board.isLegalMove(promotion));
    before.expectRestored(board);
    ASSERT_TRUE(board.makeMove(promotion));
    EXPECT_EQ(board.pieceAt(promotion.to)->getKind(), ce::PieceKind::knight);
    EXPECT_TRUE(board.getHistory().last().captured);
}

TEST(BoardUndo, RestoresPositionAfterRejectedKingExposure) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'e', 1, ce::white);
    test::put<ce::King>(board, 'a', 8, ce::black);
    test::put<ce::Rook>(board, 'e', 2, ce::white);
    test::put<ce::Rook>(board, 'e', 8, ce::black);
    const BoardSnapshot before(board);

    EXPECT_FALSE(board.isLegalMove(test::move('e', 2, 'd', 2)));
    before.expectRestored(board);
}

TEST(BoardUndo, SearchRestoresTheEntirePosition) {
    ce::Game game;
    ASSERT_TRUE(game.makeMove(test::move('e', 2, 'e', 4)));
    const BoardSnapshot before(game.board());
    const ce::Color side = game.sideToMove();
    const ce::GameStatus status = game.status();

    const ce::Move selected = game.getAiMove(2);
    EXPECT_GE(selected.from, 0);
    EXPECT_EQ(game.sideToMove(), side);
    EXPECT_EQ(game.status(), status);
    before.expectRestored(game.board());
}

TEST(BoardUndo, SearchRestoresPromotionCandidates) {
    auto board = test::emptyBoard();
    test::put<ce::King>(board, 'a', 1, ce::white);
    test::put<ce::King>(board, 'h', 8, ce::black);
    test::put<ce::Pawn>(board, 'a', 7, ce::white);
    test::put<ce::Rook>(board, 'b', 8, ce::black);
    ce::Game game(std::move(board));
    const BoardSnapshot before(game.board());

    EXPECT_GE(game.getAiMove(2).from, 0);
    before.expectRestored(game.board());
}
