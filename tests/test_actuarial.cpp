#include <gtest/gtest.h>
#include "Policyholder.hpp"
#include "RiskEvaluator.hpp"
#include "ActuarialEngine.hpp"
#include "MonteCarloSimulator.hpp"

TEST(PolicyholderTest, ValidationAndGetters) {
    Policyholder ph("POL-001", "testuser", "Test Client", "", "", CoverType::Life,
                    40, 5000.0, 10000.0, "Suburban", 100000.0, 1);
    EXPECT_EQ(ph.getPolicyId(), "POL-001");
    EXPECT_EQ(ph.getAge(), 40);
    EXPECT_TRUE(ph.isValid());

    Policyholder invalidPh("", "testuser", "Test Client", "", "", CoverType::Life,
                           15, -100.0, 0.0, "Rural", -500.0, -1);
    EXPECT_FALSE(invalidPh.isValid());
}

TEST(RiskEvaluatorTest, Calculations) {
    Policyholder ph("POL-002", "testuser", "Test Client", "", "", CoverType::Car,
                    30, 4000.0, 5000.0, "Urban", 200000.0, 2);
    double riskScore = RiskEvaluator::calculateRiskScore(ph);
    double expectedLoss = RiskEvaluator::calculateExpectedLoss(ph);

    EXPECT_GT(riskScore, 0.0);
    EXPECT_LE(riskScore, 100.0);
    EXPECT_GT(expectedLoss, 0.0);
}

TEST(ActuarialEngineTest, PremiumPricing) {
    Policyholder ph("POL-003", "testuser", "Test Client", "", "", CoverType::Property,
                    45, 8000.0, 20000.0, "Urban", 300000.0, 0);
    ActuarialEngine engine(0.15, 0.05);
    double basePremium = engine.calculateBasePremium(ph);
    
    MonteCarloSimulator simulator(1000, 4);
    auto losses = simulator.simulateLosses(ph);
    double var95 = simulator.calculateVaR(losses, 0.95);
    double riskAdjustedPremium = engine.calculateRiskAdjustedPremium(ph, var95);

    EXPECT_GT(basePremium, 0.0);
    EXPECT_GT(riskAdjustedPremium, basePremium);
}

TEST(MonteCarloSimulatorTest, SimulationAndVaR) {
    Policyholder ph("POL-004", "testuser", "Test Client", "", "", CoverType::AssetProtection,
                    50, 6000.0, 15000.0, "Suburban", 150000.0, 1);
    MonteCarloSimulator simulator(500, 4);
    auto losses = simulator.simulateLosses(ph);

    EXPECT_EQ(losses.size(), 500);
    double var95 = simulator.calculateVaR(losses, 0.95);
    EXPECT_GE(var95, 0.0);
}
