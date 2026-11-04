#ifndef AUTH_MANAGER_HPP
#define AUTH_MANAGER_HPP

#include <string>

class AuthManager {
public:
    static bool registerUser(const std::string& username, const std::string& password);
    static bool loginUser(const std::string& username, const std::string& password);
};

#endif // AUTH_MANAGER_HPP