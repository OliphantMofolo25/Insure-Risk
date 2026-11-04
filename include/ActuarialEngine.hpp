#ifndef ACTUARIALENGINE_HPP
#define ACTUARIALENGINE_HPP

#include "Policyholder.hpp"

class ActuarialEngine {
private:
    double expenseLoadingRatio;
    double profitMarginRatio;

public:
    ActuarialEngine(double expenseLoading = 0.15, double profitMargin = 0.05);
    double calculateBasePremium(const Policyholder& policyholder) const;
    double calculateRiskAdjustedPremium(const Policyholder& policyholder, double var95Loss) const;
    double getExpenseLoading() const;
    double getProfitMargin() const;
};

#endif // ACTUARIALENGINE_HPP
