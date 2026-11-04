#include "RiskAdvisoryEngine.hpp"

QVector<MitigationMeasure> RiskAdvisoryEngine::suggestions(double riskScore) {
    const double scale = riskScore >= 55.0 ? 1.25 : 1.0;
    return {{"Install smart alarms", "Improves security response", 0.8 * scale, 800.0 * scale},
            {"Retrofit flood barriers", "Reduces water damage exposure", 1.2 * scale, 650.0 * scale},
            {"Reinforce roof fixings", "Improves storm resilience", 1.4 * scale, 540.0 * scale},
            {"Winterise roof drainage", "Reduces seasonal water risk", 0.7 * scale, 160.0 * scale}};
}

MitigationOutcome RiskAdvisoryEngine::combine(const QVector<MitigationMeasure>& selected) {
    MitigationOutcome outcome;
    for (const auto& item : selected) { outcome.combinedRiskReduction += item.riskReduction; outcome.combinedAnnualSaving += item.annualSaving; }
    return outcome;
}
