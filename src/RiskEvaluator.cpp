#include "RiskEvaluator.hpp"
#include <algorithm>

double RiskEvaluator::calculateRiskScore(const Policyholder& ph) {
    double coverMultiplier = 1.0;
    switch (ph.getCoverType()) {
        case CoverType::Life: coverMultiplier = 1.1; break;
        case CoverType::Car: coverMultiplier = 1.3; break;
        case CoverType::Property: coverMultiplier = 0.9; break;
        case CoverType::AssetProtection: coverMultiplier = 1.0; break;
    }

    double dti = ph.calculateDebtToIncomeRatio();
    double zoneFactor = (ph.getGeographicZone() == "Urban") ? 1.25 : 1.0;
    double specificRisk = static_cast<double>(ph.getRiskParam()) * 5.0;

    double baseScore = (ph.getAge() * 0.4) + (dti * 20.0) + specificRisk;
    double finalScore = baseScore * coverMultiplier * zoneFactor;

    if (finalScore > 100.0) finalScore = 100.0;
    if (finalScore < 1.0) finalScore = 1.0;

    return finalScore;
}

double RiskEvaluator::calculateExpectedLoss(const Policyholder& ph) {
    double riskScore = calculateRiskScore(ph);
    return ph.getAssetValue() * (riskScore / 200.0);
}
