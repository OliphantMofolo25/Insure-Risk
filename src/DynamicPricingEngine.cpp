#include "DynamicPricingEngine.hpp"
#include <QLocale>
#include <algorithm>

PricingResult DynamicPricingEngine::calculate(const PricingRequest& request) {
    PricingResult result;
    const double loading = request.expectedLoss * request.riskLoadingPercent / 100.0;
    result.costOfCapital = std::max(0.0, request.var95 - request.expectedLoss) * 0.06;
    const double beforeMargin = request.expectedLoss + loading + result.costOfCapital + 150.0;
    result.targetMargin = beforeMargin * request.marginPercent / 100.0;
    result.baselineAnnualPremium = beforeMargin + result.targetMargin;
    const QVector<QPair<QString,double>> definitions = {{"Low deductible", 5.0}, {"Medium deductible", 12.5}, {"High deductible", 25.0}};
    for (int i = 0; i < definitions.size(); ++i) {
        TierQuote tier;
        tier.name = definitions[i].first;
        tier.deductiblePercent = std::clamp(definitions[i].second + request.deductibleAdjustmentPercent, 0.0, 45.0);
        tier.annualPremium = result.baselineAnnualPremium * (1.0 - tier.deductiblePercent / 100.0);
        tier.monthlyPremium = tier.annualPremium / 12.0;
        tier.recommended = (request.riskScore < 55.0 && i == 1) || (request.riskScore >= 55.0 && i == 2);
        result.tiers.push_back(tier);
    }
    return result;
}

QString DynamicPricingEngine::money(double amount) { return "M " + QLocale().toString(amount, 'f', 2); }
