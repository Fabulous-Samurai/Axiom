#include <gtest/gtest.h>
#include "string_helpers.h"

TEST(StringHelpersTest, FastParseDouble) {
    EXPECT_EQ(Utils::FastParseDouble("0.5").value(), 0.5);
    EXPECT_EQ(Utils::FastParseDouble(".5").value(), 0.5);
    EXPECT_EQ(Utils::FastParseDouble("5.").value(), 5.0);
    EXPECT_EQ(Utils::FastParseDouble("5.0").value(), 5.0);
    EXPECT_EQ(Utils::FastParseDouble("123.456").value(), 123.456);
    EXPECT_EQ(Utils::FastParseDouble("-4.2").value(), -4.2);

    EXPECT_FALSE(Utils::FastParseDouble("notanumber").has_value());
    EXPECT_FALSE(Utils::FastParseDouble("").has_value());
    EXPECT_FALSE(Utils::FastParseDouble("1.23a").has_value());
}

TEST(StringHelpersTest, IsNumber) {
    EXPECT_TRUE(Utils::IsNumber("0.5"));
    EXPECT_TRUE(Utils::IsNumber(".5"));
    EXPECT_TRUE(Utils::IsNumber("5."));
    EXPECT_TRUE(Utils::IsNumber("5.0"));
    EXPECT_TRUE(Utils::IsNumber("-4.2"));

    EXPECT_FALSE(Utils::IsNumber("notanumber"));
    EXPECT_FALSE(Utils::IsNumber(""));
    EXPECT_FALSE(Utils::IsNumber("1.23a"));
}
