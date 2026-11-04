#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include "Policyholder.hpp"
#include <string>

class DatabaseManager {
public:
    static void initializeDatabase();
    static bool savePolicyRecord(const Policyholder& ph, double riskScore, double expectedLoss, double var95, double annualPremium, const std::string& status);
    static bool updatePolicyStatus(int dbId, const std::string& newStatus);
    static bool setSetting(const std::string& key, const std::string& value);
    static std::string getSetting(const std::string& key, const std::string& fallback = "");
};

#endif // DATABASE_MANAGER_HPP



