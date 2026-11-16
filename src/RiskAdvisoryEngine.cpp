#include "RiskAdvisoryEngine.hpp"

QVector<MitigationMeasure> RiskAdvisoryEngine::suggestions(double riskScore, CoverType coverType) {
    const double scale = riskScore >= 55.0 ? 1.25 : 1.0;
    switch (coverType) {
    case CoverType::Life:
        return {{"Schedule preventive health checks", "Support early detection and routine care", 1.0 * scale, 900.0 * scale},
                {"Maintain prescribed treatment", "Reduce complications by following care plans", 1.4 * scale, 1100.0 * scale},
                {"Use a healthy lifestyle plan", "Lower preventable health risks through activity and nutrition", 1.1 * scale, 650.0 * scale},
                {"Review beneficiaries annually", "Keep life cover aligned with family needs", 0.5 * scale, 250.0 * scale}};
    case CoverType::Car:
        return {{"Install an approved vehicle alarm", "Deter theft and improve recovery chances", 1.2 * scale, 800.0 * scale},
                {"Use secure, well-lit parking", "Reduce theft and vandalism exposure", 1.0 * scale, 500.0 * scale},
                {"Maintain tyres and brakes", "Reduce collision risk through routine servicing", 1.5 * scale, 900.0 * scale},
                {"Complete defensive driving training", "Lower the chance and severity of road accidents", 1.3 * scale, 700.0 * scale}};
    case CoverType::Property:
        return {{"Install smoke and intruder alarms", "Improve early warning for fire and break-ins", 1.1 * scale, 800.0 * scale},
                {"Fit flood barriers and leak sensors", "Limit water damage to the building", 1.3 * scale, 650.0 * scale},
                {"Reinforce roof and windows", "Improve resilience to storms and forced entry", 1.4 * scale, 540.0 * scale},
                {"Clear gutters and drains seasonally", "Reduce flooding and roof water damage", 0.8 * scale, 300.0 * scale}};
    case CoverType::AssetProtection:
        return {{"Keep an asset inventory with photos", "Speed up verification and recovery after a loss", 0.8 * scale, 400.0 * scale},
                {"Use locks or secure storage", "Reduce theft and accidental damage exposure", 1.2 * scale, 750.0 * scale},
                {"Service valuable equipment regularly", "Prevent avoidable breakdowns", 1.3 * scale, 850.0 * scale},
                {"Install surge protection", "Protect electronics from power fluctuations", 1.0 * scale, 500.0 * scale}};
    }
    return {};
}

MitigationOutcome RiskAdvisoryEngine::combine(const QVector<MitigationMeasure>& selected) {
    MitigationOutcome outcome;
    for (const auto& item : selected) { outcome.combinedRiskReduction += item.riskReduction; outcome.combinedAnnualSaving += item.annualSaving; }
    return outcome;
}
