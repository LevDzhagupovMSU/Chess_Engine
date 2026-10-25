#include "history.hpp"

#include <gtest/gtest.h>

TEST(HistoryRules, RecordsAndIteratesMovesInOrder) {
    ce::History history;
    EXPECT_TRUE(history.empty());
    EXPECT_EQ(history.size(), 0u);

    history.record(ce::PieceKind::pawn, {52, 36}, ce::white);
    history.record(ce::PieceKind::pawn, {12, 28}, ce::black);

    ASSERT_EQ(history.size(), 2u);
    EXPECT_EQ(history.at(0).move.from, 52);
    EXPECT_EQ(history.last().move.to, 28);
    EXPECT_EQ(history.last().color, ce::black);

    auto it = history.begin();
    ASSERT_NE(it, history.end());
    EXPECT_EQ(it->color, ce::white);
    ++it;
    ASSERT_NE(it, history.end());
    EXPECT_EQ(it->color, ce::black);
}

TEST(HistoryRules, CopiesHaveIndependentRecords) {
    ce::History original;
    original.record(ce::PieceKind::pawn, {52, 36}, ce::white);

    ce::History copy(original);
    copy.record(ce::PieceKind::knight, {1, 18}, ce::black);
    ASSERT_EQ(original.size(), 1u);
    ASSERT_EQ(copy.size(), 2u);
    EXPECT_EQ(original.last().piece, ce::PieceKind::pawn);

    copy.clear();
    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(original.size(), 1u);
}
