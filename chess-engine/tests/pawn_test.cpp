#include <gtest/gtest.h>
#include "piece_test_support.hpp"

TEST(PawnRules, WhiteMovementAndCapture) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Pawn>(board, 'e', 2, white);
    ASSERT_TRUE(canMove(board, move('e', 2, 'e', 3))) << "White pawn must advance one square";
    ASSERT_TRUE(canMove(board, move('e', 2, 'e', 4))) << "White pawn must advance two squares initially";
    ASSERT_TRUE(!canMove(board, move('e', 2, 'e', 5))) << "Pawn cannot advance three squares";
    ASSERT_TRUE(!canMove(board, move('e', 2, 'e', 1))) << "Pawn cannot move backward";
    ASSERT_TRUE(!canMove(board, move('e', 2, 'd', 3))) << "Pawn cannot capture an empty square";
    put<Knight>(board, 'e', 3, black);
    ASSERT_TRUE(!canMove(board, move('e', 2, 'e', 3))) << "Pawn cannot advance into a piece";
    ASSERT_TRUE(!canMove(board, move('e', 2, 'e', 4))) << "Pawn cannot jump a piece";
    put<Knight>(board, 'd', 3, black);
    ASSERT_TRUE(canMove(board, move('e', 2, 'd', 3))) << "Pawn must capture diagonally";
    put<Knight>(board, 'f', 3, white);
    ASSERT_TRUE(!canMove(board, move('e', 2, 'f', 3))) << "Pawn cannot capture its own piece";
}

TEST(PawnRules, BlackMovement) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Pawn>(board, 'e', 7, black);
    ASSERT_TRUE(canMove(board, move('e', 7, 'e', 6))) << "Black pawn must advance one square";
    ASSERT_TRUE(canMove(board, move('e', 7, 'e', 5))) << "Black pawn must advance two squares initially";
    put<Pawn>(board, 'e', 6, white);
    ASSERT_TRUE(!canMove(board, move('e', 7, 'e', 5))) << "Black pawn cannot jump a piece";
    put<Pawn>(board, 'd', 6, white);
    ASSERT_TRUE(canMove(board, move('e', 7, 'd', 6))) << "Black pawn must capture diagonally";
}

TEST(PawnRules, WhiteEnPassant) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Pawn>(board, 'e', 5, white);
    put<Pawn>(board, 'd', 7, black);
    play(board, move('d', 7, 'd', 5));
    ASSERT_TRUE(canMove(board, move('e', 5, 'd', 6))) << "White en passant must be available immediately";
    play(board, move('e', 5, 'd', 6));
    ASSERT_TRUE(!board.pieceAt(square('d', 5))) << "En passant must remove the captured pawn";
    ASSERT_TRUE(board.pieceAt(square('d', 6)) &&
            board.pieceAt(square('d', 6))->getColor() == white) << "Capturing pawn must occupy the landing square";
    ASSERT_TRUE(board.getHistory().last().move.to == square('d', 6)) << "En passant must be recorded as the capturing move";
    EXPECT_TRUE(board.getHistory().last().captured);
}

TEST(PawnRules, EnPassantExpires) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Pawn>(board, 'e', 5, white);
    put<Pawn>(board, 'd', 7, black);
    put<Knight>(board, 'a', 1, white);
    play(board, move('d', 7, 'd', 5));
    play(board, move('a', 1, 'b', 3));
    ASSERT_TRUE(!canMove(board, move('e', 5, 'd', 6))) << "En passant expires after the next move";
}

TEST(PawnRules, BlackEnPassant) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Pawn>(board, 'd', 4, black);
    put<Pawn>(board, 'e', 2, white);
    play(board, move('e', 2, 'e', 4));
    ASSERT_TRUE(canMove(board, move('d', 4, 'e', 3))) << "Black en passant must be available";
    play(board, move('d', 4, 'e', 3));
    ASSERT_TRUE(!board.pieceAt(square('e', 4)) &&
            board.pieceAt(square('e', 3))->getColor() == black) << "Black en passant must remove the white pawn";
}

TEST(PawnRules, EnPassantCannotExposeKing) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<King>(board, 'h', 5, white);
    put<Rook>(board, 'a', 5, black);
    put<Pawn>(board, 'e', 5, white);
    put<Pawn>(board, 'd', 7, black);
    play(board, move('d', 7, 'd', 5));
    const Move capture = move('e', 5, 'd', 6);
    ASSERT_TRUE(canMove(board, capture)) << "Pinned pawn has a pseudo-legal en passant move";
    ASSERT_TRUE(!board.isLegalMove(capture)) << "En passant is illegal when it exposes the king to check";
}

TEST(PawnRules, DefaultPromotion) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Pawn>(board, 'e', 7, white);
    int promotions = 0;
    for (const Move& candidate : board.pieceAt(square('e', 7))->FINDPOSSIBLEMOVES(board)) {
        if (candidate.to == square('e', 8)) ++promotions;
    }
    ASSERT_TRUE(promotions == 4) << "Pawn must offer four promotion choices";
    play(board, move('e', 7, 'e', 8));
    ASSERT_TRUE(board.pieceAt(square('e', 8))->getKind() == PieceKind::queen &&
            board.pieceAt(square('e', 8))->getPrice() == 900 &&
            board.getHistory().last().piece == PieceKind::pawn) << "Pawn must promote to a queen by default";
}

TEST(PawnRules, CaptureUnderpromotion) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Pawn>(board, 'e', 7, white);
    put<Rook>(board, 'f', 8, black);
    const Move promotion{square('e', 7), square('f', 8), PieceKind::knight};
    ASSERT_TRUE(canMove(board, promotion)) << "Pawn must be able to capture and underpromote";
    play(board, promotion);
    ASSERT_TRUE(board.pieceAt(square('f', 8))->getKind() == PieceKind::knight &&
            board.pieceAt(square('f', 8))->getPrice() == 320 &&
            !board.pieceAt(square('e', 7))) << "Capture promotion must replace the pawn with the chosen piece";
}

TEST(PawnRules, BlackUnderpromotion) {
    using namespace ce;
    using namespace test;

    auto board = emptyBoard();
    put<Pawn>(board, 'a', 2, black);
    ASSERT_TRUE(!canMove(board, {square('a', 2), square('a', 1), PieceKind::king})) << "Pawn cannot promote to a king";
    const Move promotion{square('a', 2), square('a', 1), PieceKind::rook};
    play(board, promotion);
    ASSERT_TRUE(board.pieceAt(square('a', 1))->getKind() == PieceKind::rook &&
            board.pieceAt(square('a', 1))->getColor() == black) << "Black pawn must be able to underpromote";
}
