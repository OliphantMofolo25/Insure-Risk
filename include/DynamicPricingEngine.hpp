#pragma once
#include <QString>
#include <QVector>
#include <QMetaType>

struct TierQuote {
    QString name;
    double deductiblePercent = 0.0;
    double annualPremium = 0.0;
    double monthlyPremium = 0.0;
    bool recommended = false;
};

struct PricingRequest {
    double expectedLoss = 0.0;
    double var95 = 0.0;
    double riskScore = 0.0;
    double marginPercent = 20.0;
    double riskLoadingPercent = 25.0;
    double deductibleAdjustmentPercent = 0.0;
};

struct PricingResult {
    double baselineAnnualPremium = 0.0;
    double costOfCapital = 0.0;
    double targetMargin = 0.0;
    QVector<TierQuote> tiers;
};

class DynamicPricingEngine {
public:
    static PricingResult calculate(const PricingRequest& request);
    static QString money(double amount);
};
Q_DECLARE_METATYPE(TierQuote)
Q_DECLARE_METATYPE(PricingRequest)
Q_DECLARE_METATYPE(PricingResult)


