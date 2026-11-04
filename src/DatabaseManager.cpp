#include "DatabaseManager.hpp"
#include <sqlite3.h>
#include <iostream>

void DatabaseManager::initializeDatabase() {
    sqlite3* db = nullptr;
    char* errMsg = nullptr;

    int rc = sqlite3_open("insurerisk.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    const char* sqlUsers = "CREATE TABLE IF NOT EXISTS users ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                           "username TEXT UNIQUE NOT NULL, "
                           "password TEXT NOT NULL);";
sqlite3_exec(db, sqlUsers, nullptr, 0, &errMsg);
    sqlite3_exec(db, "ALTER TABLE users ADD COLUMN password_hash TEXT;", nullptr, 0, nullptr);
    sqlite3_exec(db, "ALTER TABLE users ADD COLUMN password_salt TEXT;", nullptr, 0, nullptr);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS app_settings (setting_key TEXT PRIMARY KEY, setting_value TEXT NOT NULL);", nullptr, 0, &errMsg);

    // Use CREATE TABLE IF NOT EXISTS to preserve saved records across application launches
    const char* sqlPolicies = "CREATE TABLE IF NOT EXISTS policies ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "username TEXT NOT NULL, "
                              "policy_id TEXT NOT NULL, "
                              "client_name TEXT NOT NULL, "
                              "phone_number TEXT NOT NULL, "
                              "address TEXT NOT NULL, "
                              "cover_type TEXT NOT NULL, "
                              "asset_value REAL NOT NULL, "
                              "risk_score REAL NOT NULL, "
                              "expected_loss REAL NOT NULL, "
                              "var_95 REAL NOT NULL, "
                              "annual_premium REAL NOT NULL, "
                              "monthly_premium REAL NOT NULL, "
                              "status TEXT NOT NULL);";

    rc = sqlite3_exec(db, sqlPolicies, nullptr, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating policies table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
}

bool DatabaseManager::savePolicyRecord(const Policyholder& ph, double riskScore, double expectedLoss, double var95, double annualPremium, const std::string& status) {
    sqlite3* db = nullptr;
    if (sqlite3_open("insurerisk.db", &db) != SQLITE_OK) {
        std::cerr << "Database open failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string sql = "INSERT INTO policies (username, policy_id, client_name, phone_number, address, cover_type, asset_value, risk_score, expected_loss, var_95, annual_premium, monthly_premium, status) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    std::string coverStr;
    switch (ph.getCoverType()) {
        case CoverType::Life: coverStr = "Life Insurance"; break;
        case CoverType::Car: coverStr = "Car Insurance"; break;
        case CoverType::Property: coverStr = "Property Insurance"; break;
        case CoverType::AssetProtection: coverStr = "Asset Protection"; break;
    }

    double monthlyPremium = annualPremium / 12.0;

    sqlite3_bind_text(stmt, 1, ph.getUsername().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ph.getPolicyId().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ph.getClientName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, ph.getPhoneNumber().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, ph.getAddress().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, coverStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 7, ph.getAssetValue());
    sqlite3_bind_double(stmt, 8, riskScore);
    sqlite3_bind_double(stmt, 9, expectedLoss);
    sqlite3_bind_double(stmt, 10, var95);
    sqlite3_bind_double(stmt, 11, annualPremium);
    sqlite3_bind_double(stmt, 12, monthlyPremium);
    sqlite3_bind_text(stmt, 13, status.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQLite step error during insert: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

bool DatabaseManager::setSetting(const std::string& key, const std::string& value) {
    sqlite3* db = nullptr;
    if (sqlite3_open("insurerisk.db", &db) != SQLITE_OK) return false;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO app_settings(setting_key, setting_value) VALUES(?, ?) ON CONFLICT(setting_key) DO UPDATE SET setting_value=excluded.setting_value;";
    const bool prepared = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK;
    if (!prepared) { sqlite3_close(db); return false; }
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
    const bool saved = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt); sqlite3_close(db); return saved;
}

std::string DatabaseManager::getSetting(const std::string& key, const std::string& fallback) {
    sqlite3* db = nullptr;
    if (sqlite3_open("insurerisk.db", &db) != SQLITE_OK) return fallback;
    sqlite3_stmt* stmt = nullptr; std::string value = fallback;
    if (sqlite3_prepare_v2(db, "SELECT setting_value FROM app_settings WHERE setting_key=?;", -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) { const auto* text = sqlite3_column_text(stmt, 0); if (text) value = reinterpret_cast<const char*>(text); }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db); return value;
}

bool DatabaseManager::updatePolicyStatus(int dbId, const std::string& newStatus) {
    if (newStatus != "Approved" && newStatus != "Rejected") return false;
    sqlite3* db = nullptr;
    if (sqlite3_open("insurerisk.db", &db) != SQLITE_OK) return false;
    sqlite3_stmt* stmt = nullptr;
    const bool ready = sqlite3_prepare_v2(db, "UPDATE policies SET status=? WHERE id=?;", -1, &stmt, nullptr) == SQLITE_OK;
    if (!ready) { sqlite3_close(db); return false; }
    sqlite3_bind_text(stmt, 1, newStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, dbId);
    const bool changed = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) == 1;
    sqlite3_finalize(stmt); sqlite3_close(db); return changed;
}
