#ifndef POLICYHOLDER_HPP
#define POLICYHOLDER_HPP

#include <string>

enum class CoverType {
    Life = 0,
    Car = 1,
    Property = 2,
    AssetProtection = 3
};

class Policyholder {
private:
    std::string policyId;
    std::string username;
    std::string clientName;
    std::string phoneNumber;
    std::string address;
    CoverType coverType;
    int age;
    double monthlyIncome;
    double activeLoans;
    std::string geographicZone;
    double assetValue;
    int riskParam;

public:
    Policyholder(std::string pId, std::string user, std::string cName, std::string phone, std::string addr,
                 CoverType cover, int a, double income, double loans, std::string geo, double asset, int rParam);

    std::string getPolicyId() const;
    std::string getUsername() const;
    std::string getClientName() const;
    std::string getPhoneNumber() const;
    std::string getAddress() const;
    CoverType getCoverType() const;
    int getAge() const;
    double getMonthlyIncome() const;
    double getActiveLoans() const;
    std::string getGeographicZone() const;
    double getAssetValue() const;
    int getRiskParam() const;

    double calculateDebtToIncomeRatio() const;
    bool isValid() const;
};

#endif // POLICYHOLDER_HPP
