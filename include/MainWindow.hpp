#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QProgressBar>
#include <QTabWidget>
#include <QVector>

#include "Policyholder.hpp"
#include "DynamicPricingEngine.hpp"
#include "RiskAdvisoryEngine.hpp"

class QChartView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleLogin();
    void handleRegister();
    void handleLogout();
    void runSimulation();
    void acceptApplication();
    void rejectApplication();
    void refreshDatabaseTable();

private:
    void setupUI();

    QStackedWidget *centralStack;
    QWidget *loginScreen;
    QWidget *appScreen;

    QLineEdit *userLoginInput;
    QLineEdit *passLoginInput;
    QLabel *authStatusLabel;

    QTabWidget *tabWidget;

    QLineEdit *policyIdInput;
    QLineEdit *clientNameInput;
    QLineEdit *phoneInput;
    QLineEdit *addressInput;
    QComboBox *coverTypeCombo;
    QLineEdit *ageInput;
    QLineEdit *incomeInput;
    QLineEdit *loansInput;
    QComboBox *geoZoneCombo;
    QLineEdit *assetInput;
    QLineEdit *riskParamInput;
    QLabel *riskParamLabel;

    QTextEdit *resultsOutput;
    QProgressBar *simProgressBar;
    QPushButton *acceptBtn;
    QPushButton *rejectBtn;
    QPushButton *referBtn = nullptr;
    QPushButton *quoteBtn = nullptr;
    QVector<QProgressBar *> riskDriverBars;
    QString latestReport;
    double latestScore = 0.0;
    double latestExpectedLoss = 0.0;
    double latestRiskLoadedLoss = 0.0;
    double latestCapitalCharge = 0.0;
    double latestAnnualPremium = 0.0;
    double latestMonthlyPremium = 0.0;

    QTableWidget *recordsTable;
    QPushButton *refreshBtn;

    QLabel *analyticsSummaryLabel;
    QFrame *riskScoreKpi = nullptr;
    QFrame *expectedLossKpi = nullptr;
    QFrame *varKpi = nullptr;
    QFrame *monthlyPremiumKpi = nullptr;
    QLabel *decisionBadge = nullptr;
    bool darkTheme = false;
    PricingResult latestPricing;
    QVector<MitigationMeasure> mitigationMeasures;

    // Qt 6 Charts view
    QChartView *chartView = nullptr;

    QString currentUser;
};

#endif // MAIN_WINDOW_HPP










