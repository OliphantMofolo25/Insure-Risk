#include <QApplication>
#include <iostream>
#include <limits>
#include <string>
#include "MainWindow.hpp"
#include "DatabaseManager.hpp"
#include "AuthManager.hpp"

namespace {
int runCli() {
    std::cout << "\nMatseliso_InsureRisk - CLI mode\n";
    std::cout << "Use --qt to launch the desktop interface instead.\n\n";
    for (;;) {
        std::cout << "1) Register\n2) Log in\n0) Exit\nChoose: ";
        int choice = -1;
        if (!(std::cin >> choice)) return 1;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (choice == 0) return 0;
        if (choice != 1 && choice != 2) { std::cout << "Please choose 1, 2, or 0.\n\n"; continue; }
        std::string username, password;
        std::cout << "Username: "; std::getline(std::cin, username);
        std::cout << "Password: "; std::getline(std::cin, password);
        const bool ok = choice == 1 ? AuthManager::registerUser(username, password) : AuthManager::loginUser(username, password);
        if (choice == 1) std::cout << (ok ? "Registration complete. You can now log in.\n\n" : "Registration failed: username may already exist.\n\n");
        else if (ok) { std::cout << "Authenticated successfully as " << username << ".\n"; return 0; }
        else std::cout << "Login failed. Check your credentials.\n\n";
    }
}
}

int main(int argc, char *argv[]) {
    DatabaseManager::initializeDatabase();
    if (argc > 1 && std::string(argv[1]) == "--cli") return runCli();
    if (argc > 1 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: InsureRiskLinux [--qt | --cli]\n";
        return 0;
    }
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
