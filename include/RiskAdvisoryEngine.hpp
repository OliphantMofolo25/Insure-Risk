#pragma once
#include <QString>
#include <QVector>
#include <QMetaType>
#include "Policyholder.hpp"

struct MitigationMeasure {
    QString name;
    QString description;
    double riskReduction = 0.0;
    double annualSaving = 0.0;
};

struct MitigationOutcome {
    double combinedRiskReduction = 0.0;
    double combinedAnnualSaving = 0.0;
};

class RiskAdvisoryEngine {
public:
    static QVector<MitigationMeasure> suggestions(double riskScore, CoverType coverType = CoverType::Property);
    static MitigationOutcome combine(const QVector<MitigationMeasure>& selected);
};
Q_DECLARE_METATYPE(MitigationMeasure)
Q_DECLARE_METATYPE(MitigationOutcome)


