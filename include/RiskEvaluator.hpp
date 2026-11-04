#ifndef RISK_EVALUATOR_HPP
#define RISK_EVALUATOR_HPP

#include "Policyholder.hpp"

class RiskEvaluator {
public:
    static double calculateRiskScore(const Policyholder& ph);
    static double calculateExpectedLoss(const Policyholder& ph);
};

#endif // RISK_EVALUATOR_HPP