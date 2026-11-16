#include <gtest/gtest.h>
#include <string>

TEST(InsuranceRiskSuite, BasicSanityCheck) {
    EXPECT_EQ(5 + 5, 10);
}

TEST(InsuranceRiskSuite, ValidAgeBoundary) {
    int age = 25;
    EXPECT_GE(age, 18);
    EXPECT_LT(-5, 0);
}

TEST(InsuranceRiskSuite, RiskCategoryCheck) {
    std::string risk = "High";
    EXPECT_EQ(risk, "High");
}
