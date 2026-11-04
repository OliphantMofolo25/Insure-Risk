#include "ActuarialEngine.hpp"
#include "RiskEvaluator.hpp"

ActuarialEngine::ActuarialEngine(double expenseLoading, double profitMargin)
    : expenseLoadingRatio(expenseLoading), profitMarginRatio(profitMargin) {}

double ActuarialEngine::calculateBasePremium(const Policyholder& policyholder) const {
    double expectedLoss = RiskEvaluator::calculateExpectedLoss(policyholder);
    // Standard actuarial pricing formula: Pure Premium + Expense Loading + Profit Margin
    return expectedLoss * (1.0 + expenseLoadingRatio + profitMarginRatio);
}

double ActuarialEngine::calculateRiskAdjustedPremium(const Policyholder& policyholder, double var95Loss) const {
    double basePremium = calculateBasePremium(policyholder);
    double expectedLoss = RiskEvaluator::calculateExpectedLoss(policyholder);
    
    // Add a capital surcharge based on tail risk (VaR 95% stress buffer)
    double tailRiskFactor = (expectedLoss > 0.0) ? (var95Loss / expectedLoss) * 0.04 : 0.0;
    return basePremium * (1.0 + tailRiskFactor);
}

double ActuarialEngine::getExpenseLoading() const { return expenseLoadingRatio; }
double ActuarialEngine::getProfitMargin() const { return profitMarginRatio; }