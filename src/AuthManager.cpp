#include "AuthManager.hpp"
#include "sqlite3.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QString>

namespace {
QString hashPassword(const QString& password, const QString& salt) {
    QByteArray value = salt.toUtf8() + password.toUtf8();
    for (int round = 0; round < 120000; ++round)
        value = QCryptographicHash::hash(value, QCryptographicHash::Sha256);
    return QString::fromLatin1(value.toHex());
}
QString createSalt() {
    QByteArray salt(24, Qt::Uninitialized);
    for (int i = 0; i < salt.size(); ++i) salt[i] = static_cast<char>(QRandomGenerator::global()->generate() & 0xff);
    return QString::fromLatin1(salt.toHex());
}
}

bool AuthManager::registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) return false;
    sqlite3* db = nullptr;
    if (sqlite3_open("insurerisk.db", &db) != SQLITE_OK) return false;
    const QString salt = createSalt(), hash = hashPassword(QString::fromStdString(password), salt);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO users(username, password, password_hash, password_salt) VALUES(?, '', ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) { sqlite3_close(db); return false; }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt); sqlite3_close(db); return ok;
}

bool AuthManager::loginUser(const std::string& username, const std::string& password) {
    sqlite3* db = nullptr;
    if (sqlite3_open("insurerisk.db", &db) != SQLITE_OK) return false;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT password, password_hash, password_salt FROM users WHERE username=?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) { sqlite3_close(db); return false; }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const QString legacy = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        const QString hash = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        const QString salt = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        ok = !hash.isEmpty() ? hashPassword(QString::fromStdString(password), salt) == hash : legacy == QString::fromStdString(password);
        if (ok && hash.isEmpty()) {
            const QString newSalt = createSalt(), newHash = hashPassword(QString::fromStdString(password), newSalt);
            sqlite3_stmt* update = nullptr;
            if (sqlite3_prepare_v2(db, "UPDATE users SET password='', password_hash=?, password_salt=? WHERE username=?;", -1, &update, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(update, 1, newHash.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(update, 2, newSalt.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(update, 3, username.c_str(), -1, SQLITE_TRANSIENT); sqlite3_step(update); sqlite3_finalize(update);
            }
        }
    }
    sqlite3_finalize(stmt); sqlite3_close(db); return ok;
}
