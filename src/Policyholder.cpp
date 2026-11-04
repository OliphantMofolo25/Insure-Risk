#include "Policyholder.hpp"

Policyholder::Policyholder(std::string pId, std::string user, std::string cName, std::string phone, std::string addr,
                           CoverType cover, int a, double income, double loans, std::string geo, double asset, int rParam)
    : policyId(pId), username(user), clientName(cName), phoneNumber(phone), address(addr),
      coverType(cover), age(a), monthlyIncome(income), activeLoans(loans),
      geographicZone(geo), assetValue(asset), riskParam(rParam) {}

std::string Policyholder::getPolicyId() const { return policyId; }
std::string Policyholder::getUsername() const { return username; }
std::string Policyholder::getClientName() const { return clientName; }
std::string Policyholder::getPhoneNumber() const { return phoneNumber; }
std::string Policyholder::getAddress() const { return address; }
CoverType Policyholder::getCoverType() const { return coverType; }
int Policyholder::getAge() const { return age; }
double Policyholder::getMonthlyIncome() const { return monthlyIncome; }
double Policyholder::getActiveLoans() const { return activeLoans; }
std::string Policyholder::getGeographicZone() const { return geographicZone; }
double Policyholder::getAssetValue() const { return assetValue; }
int Policyholder::getRiskParam() const { return riskParam; }

double Policyholder::calculateDebtToIncomeRatio() const {
    if (monthlyIncome <= 0.0) return 1.0;
    return activeLoans / (monthlyIncome * 12.0);
}

bool Policyholder::isValid() const {
    return !policyId.empty() && !clientName.empty() && age >= 18 && monthlyIncome >= 0 && assetValue > 0;
}
