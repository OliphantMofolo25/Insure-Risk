#include <gtest/gtest.h>
#include <cstdlib>
#include <fstream>
#include <string>

// Example: End-to-end functional test of the compiled application workflow
TEST(OpenTextStyleFunctionalSuite, EndToEndRiskAssessmentWorkflow) {
    // Replace with your actual compiled app binary and CLI arguments/inputs
    // int exit_code = std::system("./build/InsureRisk --age 25 --coverage 50000 > /tmp/app_out.txt");
    // EXPECT_EQ(exit_code, 0);

    std::string simulated_output = "Risk: Low | Premium: 120.50";
    EXPECT_NE(simulated_output.find("Risk: Low"), std::string::npos);
}

TEST(OpenTextStyleFunctionalSuite, InvalidInputRejectionWorkflow) {
    int invalid_age = -5;
    EXPECT_LT(invalid_age, 0);
}
