#include "serializer.hpp"

#include <gtest/gtest.h>
#include <string>

TEST(SerializerRules, PromotionChoiceSurvivesRoundTrip) {
    auto promoted = Serializer::from_json(
        std::string{R"({"type":"player","move":[12,4],"promotion":"knight"})"});
    ASSERT_TRUE(promoted);
    EXPECT_EQ(promoted->move.from, 12);
    EXPECT_EQ(promoted->move.to, 4);
    EXPECT_EQ(promoted->move.promotion, ce::PieceKind::knight);
    EXPECT_EQ(Serializer::to_json(*promoted).value("promotion", ""), "knight");
}

TEST(SerializerRules, TwoCoordinateMessageDefaultsToQueen) {
    auto regular = Serializer::from_json(std::string{R"({"type":"player","move":[52,36]})"});
    ASSERT_TRUE(regular);
    EXPECT_EQ(regular->move.promotion, ce::PieceKind::queen);
    EXPECT_FALSE(Serializer::to_json(*regular).contains("promotion"));
}

TEST(SerializerRules, RejectsInvalidPromotion) {
    auto invalid = Serializer::from_json(
        std::string{R"({"type":"player","move":[12,4],"promotion":"king"})"});
    ASSERT_TRUE(invalid);
    EXPECT_EQ(invalid->from(), -1);
}
