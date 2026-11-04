#include "MainWindow.hpp"
#include "AuthManager.hpp"
#include "DatabaseManager.hpp"
#include "Policyholder.hpp"
#include "RiskEvaluator.hpp"
#include "ActuarialEngine.hpp"
#include "MonteCarloSimulator.hpp"
#include "DynamicPricingEngine.hpp"
#include "RiskAdvisoryEngine.hpp"
#include "sqlite3.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QFrame>
#include <QPixmap>
#include <QThread>
#include <QCoreApplication>
#include <QPainter>
#include <QColor>
#include <QTableWidgetItem>
#include <QCheckBox>
#include <algorithm>
#include <QFileDialog>
#include <QSaveFile>
#include <QTextStream>
#include <QPdfWriter>
#include <QInputDialog>
#include <QPageSize>
#include <QSlider>
#include <QCheckBox>
#include <algorithm>
#include <QScrollArea>
#include <QAbstractItemView>
#include <QStringList>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QLegend>
#include <string>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Matseliso_InsureRisk - Enterprise Actuarial Portal");
    resize(1440, 900);
    setMinimumSize(1100, 720);
    setupUI();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    setStyleSheet(R"(
      * { font-family: "Segoe UI", Inter, "DejaVu Sans"; }
      QMainWindow, QWidget { background:#eef2f6; color:#1b2430; font-size:13px; }
      QLabel { color:#1b2430; background:transparent; }
      QFrame#card, QFrame#kpi { background:#ffffff; border:1px solid #d9e1ea; border-radius:16px; }      QFrame#homeHero { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #166534,stop:1 #16a34a); border:0; border-radius:22px; }
      QFrame#homeHero QLabel { color:#ffffff; }
      QFrame#homeService { background:#ffffff; border:1px solid #d9e1ea; border-radius:20px; }
      QLabel#homeIcon { background:#dcfce7; color:#15803d; border-radius:22px; padding:12px; font-size:17px; font-weight:700; }
      QLabel#homeCardTitle { font-size:17px; font-weight:700; }
      QPushButton#homeAction { background:#ffffff; color:#15803d; border:1px solid #22c55e; border-radius:8px; padding:9px 14px; font-weight:700; }
      QPushButton#homeAction:hover { background:#16a34a; color:#ffffff; border-color:#16a34a; }
      QPushButton#serviceCard { background:#ffffff; color:#1b2430; border:1px solid #d9e1ea; border-radius:18px; padding:22px; text-align:left; font-size:15px; font-weight:700; }
      QPushButton#serviceCard:hover, QPushButton#serviceCard:focus { background:#f0fdf4; border:2px solid #16a34a; color:#15803d; }
      QFrame#brandPanel { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #166534,stop:1 #16a34a); border:0; border-radius:14px; }
      QFrame#brandPanel QLabel { color:white; }
      QLabel#title { font-size:17px; font-weight:700; }
      QLabel#caption { color:#64748b; font-size:11px; font-weight:700; }
      QLabel#value { font-size:24px; font-weight:700; }
      QLabel#muted { color:#64748b; }
      QLabel#modelRow { background:#f3f6fa; padding:11px; border-radius:7px; }
      QLabel#pill { background:#dcfce7; color:#15803d; border-radius:18px; padding:8px 18px; font-weight:700; }
      QLineEdit, QComboBox, QTextEdit { background:#ffffff; color:#1b2430; border:1px solid #d9e1ea; border-radius:7px; padding:8px; min-height:20px; }
      QLineEdit:focus, QComboBox:focus { border:2px solid #16a34a; padding:7px; }
      QPushButton { background:#ffffff; color:#1b2430; border:1px solid #d9e1ea; border-radius:7px; padding:10px 16px; font-weight:700; min-height:18px; }
      QPushButton#GreenPrimaryBtn { background:#16a34a; color:white; border-color:#16a34a; }
      QPushButton#GreenPrimaryBtn:hover { background:#15803d; }
      QPushButton#YellowSecondaryBtn { background:#eab308; color:#1b2430; border-color:#eab308; }
      QPushButton#RedDangerBtn { background:#dc2626; color:white; border-color:#dc2626; }
      QPushButton#BackBtn { background:#ffffff; color:#1b2430; border-color:#d9e1ea; }
      QTabWidget::pane { border:0; background:#eef2f6; }
      QTabBar::tab { background:transparent; color:#64748b; border:0; padding:14px 28px; font-size:14px; font-weight:700; }
      QTabBar::tab:selected { color:#16a34a; border-bottom:3px solid #16a34a; }
      QTableWidget { background:#ffffff; color:#1b2430; alternate-background-color:#f3f6fa; border:1px solid #d9e1ea; border-radius:8px; gridline-color:transparent; selection-background-color:#dcfce7; selection-color:#1b2430; }
      QTableWidget::item { color:#1b2430; }
      QHeaderView::section { background:#f3f6fa; color:#1b2430; border:0; border-bottom:1px solid #d9e1ea; padding:10px; font-weight:700; }
      QProgressBar { border:0; background:#e2e8f0; border-radius:7px; height:15px; }
      QProgressBar::chunk { background:#16a34a; border-radius:7px; }
      QScrollArea { border:0; background:transparent; }
    )");

    auto card = [](const QString &name = QString()) {
        auto *f = new QFrame; f->setObjectName(name.isEmpty() ? "card" : name); return f;
    };
    auto text = [](const QString &value, const QString &name = QString()) {
        auto *l = new QLabel(value); if (!name.isEmpty()) l->setObjectName(name); return l;
    };
    auto field = [&text](const QString &label, QWidget *input) {
        auto *box = new QWidget; auto *l = new QVBoxLayout(box); l->setContentsMargins(0,0,0,0); l->setSpacing(5);
        l->addWidget(text(label, "caption")); l->addWidget(input); return box;
    };
    auto kpi = [&card, &text](const QString &caption, const QString &value, const QString &sub, const QString &colour = QString()) {
        auto *f = card("kpi"); auto *l = new QVBoxLayout(f); l->setContentsMargins(18,15,18,15); l->setSpacing(3);
        l->addWidget(text(caption.toUpper(), "caption")); auto *v=text(value,"value"); if (!colour.isEmpty()) v->setStyleSheet("color:"+colour+";"); l->addWidget(v); l->addWidget(text(sub,"muted")); return f;
    };

    centralStack = new QStackedWidget(this); setCentralWidget(centralStack);

    // Login screen
    loginScreen = new QWidget; auto *loginOuter = new QVBoxLayout(loginScreen); loginOuter->setContentsMargins(0,0,0,0);
    auto *loginRow = new QHBoxLayout; loginRow->setContentsMargins(90,65,90,65); loginRow->setSpacing(28);
    auto *loginCard = card(); auto *loginLayout = new QVBoxLayout(loginCard); loginLayout->setContentsMargins(42,42,42,34); loginLayout->setSpacing(12);
    loginLayout->addWidget(text("Welcome back", "title")); loginLayout->addWidget(text("Sign in to the Actuarial Portal", "muted")); loginLayout->addSpacing(14);
    userLoginInput = new QLineEdit; userLoginInput->setPlaceholderText("Username");
    passLoginInput = new QLineEdit; passLoginInput->setPlaceholderText("Password"); passLoginInput->setEchoMode(QLineEdit::Password);
    loginLayout->addWidget(field("USERNAME", userLoginInput)); loginLayout->addWidget(field("PASSWORD", passLoginInput));
    auto *showPassword = new QCheckBox("Show password"); loginLayout->addWidget(showPassword);
    auto *loginBtn = new QPushButton("Log in to Dashboard"); loginBtn->setObjectName("GreenPrimaryBtn");
    auto *registerBtn = new QPushButton("Register New Account"); registerBtn->setObjectName("OutlineBtn");
    loginLayout->addWidget(loginBtn); loginLayout->addWidget(text("────────────  or  ────────────", "muted")); loginLayout->addWidget(registerBtn);
    authStatusLabel = text("● Ready\nPlease log in or register to continue."); authStatusLabel->setStyleSheet("background:#ecfdf3;color:#15803d;border:1px solid #bbf7d0;border-radius:8px;padding:12px;"); loginLayout->addWidget(authStatusLabel); loginLayout->addStretch();
    auto *brand = card("brandPanel"); auto *brandLayout = new QVBoxLayout(brand); brandLayout->setContentsMargins(50,50,50,35); brandLayout->setAlignment(Qt::AlignHCenter); brandLayout->setSpacing(16);
    auto *logo = new QLabel; logo->setAlignment(Qt::AlignCenter); logo->setStyleSheet("background:white;border-radius:22px;padding:22px;"); QPixmap pix(":/src/images/logo.png"); if (!pix.isNull()) logo->setPixmap(pix.scaled(145,145,Qt::KeepAspectRatio,Qt::SmoothTransformation)); else logo->setText("MIR");
    brandLayout->addWidget(logo,0,Qt::AlignHCenter); brandLayout->addWidget(text("Matseliso_InsureRisk"),0,Qt::AlignHCenter); brandLayout->itemAt(1)->widget()->setStyleSheet("font-size:28px;font-weight:700;color:white;");
    auto *tag=text("Advanced Actuarial Risk Engine & Portfolio Management"); tag->setStyleSheet("font-size:14px;font-weight:700;color:#fde68a;"); brandLayout->addWidget(tag,0,Qt::AlignHCenter); brandLayout->addSpacing(22);
    for (const QString &s : {"✓  Monte Carlo Value-at-Risk (95%)", "✓  Automated, explainable premium pricing", "✓  Policyholder records & billing statements", "✓  Live portfolio analytics & audit trail"}) { auto *line=text(s); line->setStyleSheet("font-size:15px;color:white;padding:7px;"); brandLayout->addWidget(line); }
    brandLayout->addStretch(); auto *version=text("v2.0 · C++17 · Qt 6 · SQLite"); version->setStyleSheet("color:#bbf7d0;"); brandLayout->addWidget(version,0,Qt::AlignHCenter);
    loginRow->addWidget(loginCard,1); loginRow->addWidget(brand,1); loginOuter->addLayout(loginRow); centralStack->addWidget(loginScreen);

    // Application chrome
    appScreen = new QWidget; auto *app = new QVBoxLayout(appScreen); app->setContentsMargins(30,14,30,0); app->setSpacing(8);
    auto *header = new QHBoxLayout; auto *headLogo=new QLabel; if (!pix.isNull()) headLogo->setPixmap(pix.scaled(42,42,Qt::KeepAspectRatio,Qt::SmoothTransformation)); auto *headBrand=text("<span style='font-size:23px;font-weight:700;color:#16a34a;'>Matseliso_InsureRisk</span><br><span style='color:#64748b;'>Enterprise Actuarial Portal</span>");
    auto *homeBtn=new QPushButton("Home"); homeBtn->setObjectName("BackBtn"); auto *themeBtn=new QPushButton("☾  Dark mode"); auto *userChip=text("<b>●  admin</b><br><span style='color:#64748b;'>Administrator</span>"); auto *logoutBtn=new QPushButton("Log out"); logoutBtn->setObjectName("BackBtn");
    header->addWidget(headLogo); header->addWidget(headBrand); header->addStretch(); header->addWidget(homeBtn); header->addWidget(themeBtn); header->addWidget(userChip); header->addWidget(logoutBtn); app->addLayout(header);
    tabWidget = new QTabWidget;
    auto *workspaceStack = new QStackedWidget; app->addWidget(workspaceStack,1); centralStack->addWidget(appScreen);

    // Simulator
    auto *simTab = new QWidget; auto *simRow = new QHBoxLayout(simTab); simRow->setContentsMargins(0,10,0,8); simRow->setSpacing(16);
    auto *formCard = card(); auto *formBox = new QVBoxLayout(formCard); formBox->setContentsMargins(20,20,20,20); formBox->setSpacing(10); formBox->addWidget(text("Policy & Client Details","title"));
    policyIdInput=new QLineEdit; policyIdInput->setPlaceholderText("POL-101"); clientNameInput=new QLineEdit; clientNameInput->setPlaceholderText("Client full name"); phoneInput=new QLineEdit; phoneInput->setPlaceholderText("+266 5123 4567"); addressInput=new QLineEdit; addressInput->setPlaceholderText("Physical address"); coverTypeCombo=new QComboBox; coverTypeCombo->addItems({"Life Insurance","Car Insurance","Property Insurance","Asset Protection"}); ageInput=new QLineEdit; ageInput->setPlaceholderText("24"); incomeInput=new QLineEdit; incomeInput->setPlaceholderText("9,822.00"); loansInput=new QLineEdit; loansInput->setPlaceholderText("142,000.00"); geoZoneCombo=new QComboBox; geoZoneCombo->addItems({"Urban","Suburban","Rural"}); assetInput=new QLineEdit; assetInput->setPlaceholderText("40,000.00"); riskParamInput=new QLineEdit; riskParamInput->setPlaceholderText("2 - Secure"); riskParamLabel=text("HAZARD / SECURITY RATING","caption");
    formBox->addWidget(field("POLICY ID",policyIdInput)); formBox->addWidget(field("CLIENT FULL NAME",clientNameInput)); formBox->addWidget(field("PHONE NUMBER",phoneInput)); formBox->addWidget(field("PHYSICAL ADDRESS",addressInput)); formBox->addWidget(field("COVER TYPE",coverTypeCombo));
    auto *mini=new QGridLayout; mini->setHorizontalSpacing(12); mini->setVerticalSpacing(10); mini->addWidget(field("AGE",ageInput),0,0); mini->addWidget(field("GEOGRAPHIC ZONE",geoZoneCombo),0,1); mini->addWidget(field("MONTHLY INCOME (M)",incomeInput),1,0); mini->addWidget(field("ACTIVE LOANS (M)",loansInput),1,1); mini->addWidget(field("ASSET VALUE (M)",assetInput),2,0); mini->addWidget(field("HAZARD / SECURITY",riskParamInput),2,1); formBox->addLayout(mini);
    auto *calcBtn=new QPushButton("▶ Run Actuarial Simulation"); calcBtn->setObjectName("GreenPrimaryBtn"); auto *clearBtn=new QPushButton("Clear form"); formBox->addWidget(calcBtn); formBox->addWidget(clearBtn); formBox->addWidget(text("Inputs are validated live; invalid fields turn red.","muted"));
    auto *work = new QWidget; auto *workBox=new QVBoxLayout(work); workBox->setContentsMargins(0,0,0,0); workBox->setSpacing(12); auto *titleRow=new QHBoxLayout; titleRow->addWidget(text("Actuarial Analysis & Underwriting Decision","title")); titleRow->addStretch(); decisionBadge=text("● RECOMMEND ACCEPT","pill"); titleRow->addWidget(decisionBadge); workBox->addLayout(titleRow);
    auto *metrics=new QHBoxLayout; riskScoreKpi=kpi("Composite risk score","—","Moderate tier · /100"); expectedLossKpi=kpi("Expected annual loss","M —","20,000 trials"); varKpi=kpi("Value-at-Risk (95%)","M —","No-claim probability", "#dc2626"); monthlyPremiumKpi=kpi("Monthly premium","M —","Annual billed (12×)","#16a34a"); metrics->addWidget(riskScoreKpi); metrics->addWidget(expectedLossKpi); metrics->addWidget(varKpi); metrics->addWidget(monthlyPremiumKpi); workBox->addLayout(metrics);
    auto *report=card(); auto *reportBox=new QHBoxLayout(report); reportBox->setContentsMargins(20,18,20,18); auto *drivers=new QVBoxLayout; drivers->addWidget(text("Risk drivers (weighted points)","title")); auto driver=[this,&text,drivers](const QString &name){auto *row=new QWidget;auto *rl=new QHBoxLayout(row);rl->setContentsMargins(0,2,0,2);auto *label=text(name);label->setMinimumWidth(125);auto *bar=new QProgressBar;bar->setRange(0,25);bar->setValue(0);bar->setTextVisible(false);riskDriverBars.append(bar);rl->addWidget(label);rl->addWidget(bar,1);drivers->addWidget(row);}; driver("Age profile");driver("Debt-to-income");driver("Geographic zone");driver("Hazard / health");driver("Asset exposure"); drivers->addStretch(); drivers->addWidget(text("Loss distribution  ·  VaR 95%", "caption")); auto *pricing=new QVBoxLayout; pricing->addWidget(text("Pricing breakdown","title")); resultsOutput=new QTextEdit; resultsOutput->setReadOnly(true); resultsOutput->setPlaceholderText("Run a simulation to see the pricing breakdown and underwriting notes."); pricing->addWidget(resultsOutput,1); reportBox->addLayout(drivers,1); reportBox->addLayout(pricing,1); workBox->addWidget(report,1);
    auto *progressLine=new QHBoxLayout; progressLine->addWidget(text("Simulation progress","caption")); progressLine->addStretch(); progressLine->addWidget(text("20,000 / 20,000 trials · 100%","caption")); workBox->addLayout(progressLine); simProgressBar=new QProgressBar; simProgressBar->setRange(0,100); workBox->addWidget(simProgressBar); auto *actions=new QHBoxLayout; quoteBtn=new QPushButton("↓ Quote PDF"); quoteBtn->setEnabled(false); acceptBtn=new QPushButton("✓ Accept & Save"); acceptBtn->setObjectName("GreenPrimaryBtn"); acceptBtn->setEnabled(false); referBtn=new QPushButton("Refer"); referBtn->setObjectName("YellowSecondaryBtn"); referBtn->setEnabled(false); rejectBtn=new QPushButton("Reject"); rejectBtn->setObjectName("RedDangerBtn"); rejectBtn->setEnabled(false); actions->addWidget(quoteBtn); actions->addStretch(); actions->addWidget(rejectBtn); actions->addWidget(acceptBtn); referBtn->hide(); workBox->addLayout(actions); simRow->addWidget(formCard,4); simRow->addWidget(work,8); tabWidget->addTab(simTab,"Dashboard");


    // Pricing & quotation: deliberately compact; no live systems/telemetry panel.
    auto *pricingTab = new QWidget;
    auto *pricingLayout = new QHBoxLayout(pricingTab);
    pricingLayout->setContentsMargins(0, 10, 0, 8);
    pricingLayout->setSpacing(16);
    auto *matrixCard = card();
    auto *matrixLayout = new QVBoxLayout(matrixCard);
    matrixLayout->setContentsMargins(20, 18, 20, 18);
    auto *matrixHead = new QHBoxLayout;
    matrixHead->addWidget(text("Quote Matrix", "title"));
    matrixHead->addStretch();
    auto *generateQuotes = new QPushButton("Generate quotes"); generateQuotes->setObjectName("GreenPrimaryBtn");
    matrixHead->addWidget(generateQuotes);
    matrixLayout->addLayout(matrixHead);
    matrixLayout->addWidget(text("Three deductible options based on the latest simulation.", "muted"));
    auto *quoteTable = new QTableWidget(3, 6);
    quoteTable->setHorizontalHeaderLabels({"Tier", "Deductible", "Annual premium", "Monthly premium", "Recommendation", "Select"});
    quoteTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    quoteTable->verticalHeader()->setVisible(false);
    quoteTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    matrixLayout->addWidget(quoteTable, 1);
    auto *quoteNote = text("Run the simulation, then select Generate quotes.", "muted");
    matrixLayout->addWidget(quoteNote);
    pricingLayout->addWidget(matrixCard, 3);

    auto *advisoryCard = card();
    advisoryCard->setMinimumWidth(315);
    auto *advisoryLayout = new QVBoxLayout(advisoryCard);
    advisoryLayout->setContentsMargins(20, 18, 20, 18);
    advisoryLayout->addWidget(text("Pricing & Advisory", "title"));
    auto sliderRow = [&text, advisoryLayout](const QString &label, int initial, int minimum, int maximum, QSlider **sliderOut, QLabel **valueOut) {
        auto *labelRow = new QHBoxLayout; labelRow->addWidget(text(label, "caption")); labelRow->addStretch();
        auto *value = text(QString::number(initial) + "%"); value->setStyleSheet("color:#16a34a;font-weight:700;");
        labelRow->addWidget(value); advisoryLayout->addLayout(labelRow);
        auto *slider = new QSlider(Qt::Horizontal); slider->setRange(minimum, maximum); slider->setValue(initial);
        advisoryLayout->addWidget(slider); *sliderOut = slider; *valueOut = value;
    };
    QSlider *deductibleSlider = nullptr, *marginSlider = nullptr, *loadingSlider = nullptr;
    QLabel *deductibleValue = nullptr, *marginValue = nullptr, *loadingValue = nullptr;
    sliderRow("Deductible adjustment", 0, -10, 20, &deductibleSlider, &deductibleValue);
    sliderRow("Margin target", 20, 5, 40, &marginSlider, &marginValue);
    sliderRow("Risk loading", 25, 5, 50, &loadingSlider, &loadingValue);
    auto *resetPricing = new QPushButton("Reset");
    auto *recalculatePricing = new QPushButton("Recalculate"); recalculatePricing->setObjectName("GreenPrimaryBtn");
    auto *pricingActions = new QHBoxLayout; pricingActions->addWidget(resetPricing); pricingActions->addWidget(recalculatePricing);
    advisoryLayout->addLayout(pricingActions);
    advisoryLayout->addSpacing(8);
    auto *preview = text("QUOTE PREVIEW\nRun a simulation to calculate a premium.");
    preview->setWordWrap(true); preview->setStyleSheet("background:#ecfdf3;color:#15803d;border:1px solid #16a34a;border-radius:10px;padding:16px;font-size:14px;font-weight:700;");
    advisoryLayout->addWidget(preview);
    advisoryLayout->addStretch();
    pricingLayout->addWidget(advisoryCard, 1);
    tabWidget->addTab(pricingTab, "Pricing & Quotation");

    auto updatePricing = [this, quoteTable, quoteNote, preview, deductibleSlider, marginSlider, loadingSlider, deductibleValue, marginValue, loadingValue]() {
        deductibleValue->setText(QString::number(deductibleSlider->value()) + "%");
        marginValue->setText(QString::number(marginSlider->value()) + "%");
        loadingValue->setText(QString::number(loadingSlider->value()) + "%");
        if (latestExpectedLoss <= 0.0) { quoteNote->setText("Run the simulation first to prepare a quotation."); return; }
        PricingRequest request{latestExpectedLoss, latestExpectedLoss + latestCapitalCharge / .06, latestScore,
                               static_cast<double>(marginSlider->value()), static_cast<double>(loadingSlider->value()),
                               static_cast<double>(deductibleSlider->value())};
        latestPricing = DynamicPricingEngine::calculate(request);
        quoteTable->clearContents();
        for (int row = 0; row < latestPricing.tiers.size(); ++row) {
            const auto &tier = latestPricing.tiers[row];
            const QStringList values = {tier.name, QString::number(tier.deductiblePercent, 'f', 0) + "%",
                DynamicPricingEngine::money(tier.annualPremium), DynamicPricingEngine::money(tier.monthlyPremium),
                tier.recommended ? "Recommended" : "", "Select"};
            for (int col = 0; col < values.size(); ++col) {
                auto *item = new QTableWidgetItem(values[col]);
                if (tier.recommended) item->setBackground(QColor("#dcfce7"));
                if (col >= 1 && col <= 3) item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                quoteTable->setItem(row, col, item);
            }
        }
        const auto recommended = std::find_if(latestPricing.tiers.cbegin(), latestPricing.tiers.cend(), [](const TierQuote &tier){ return tier.recommended; });
        if (recommended != latestPricing.tiers.cend()) preview->setText("QUOTE PREVIEW\n" + DynamicPricingEngine::money(recommended->annualPremium) + " per year\n" + DynamicPricingEngine::money(recommended->monthlyPremium) + " per month\n" + recommended->name);
        mitigationMeasures = RiskAdvisoryEngine::suggestions(latestScore);
        QVector<MitigationMeasure> selected;
        for (int i = 0; i < measureChecks.size() && i < mitigationMeasures.size(); ++i) if (measureChecks[i]->isChecked()) selected.push_back(mitigationMeasures[i]);
        const auto outcome = RiskAdvisoryEngine::combine(selected);
        impact->setText(selected.isEmpty() ? "Select measures to see their combined impact." :
            QString("Combined impact: -%1 risk points; save %2 / year").arg(outcome.combinedRiskReduction, 0, 'f', 1).arg(DynamicPricingEngine::money(outcome.combinedAnnualSaving)));
        quoteNote->setText("Quotes use the completed simulation; changing sliders does not rerun it.");
    };
    connect(generateQuotes, &QPushButton::clicked, this, updatePricing);
    connect(recalculatePricing, &QPushButton::clicked, this, updatePricing);
    connect(resetPricing, &QPushButton::clicked, this, [deductibleSlider, marginSlider, loadingSlider, updatePricing](){ deductibleSlider->setValue(0); marginSlider->setValue(20); loadingSlider->setValue(25); updatePricing(); });
    // Policy records
    auto *recordsTab=new QWidget; auto *recordsBox=new QVBoxLayout(recordsTab); recordsBox->setContentsMargins(0,10,0,8); auto *toolbar=card(); auto *toolbarRow=new QHBoxLayout(toolbar); toolbarRow->setContentsMargins(14,12,14,12); auto *search=new QLineEdit; search->setPlaceholderText("⌕ Search ID, name, phone, address…"); auto *status=new QComboBox; status->addItem("All statuses"); auto *cover=new QComboBox; cover->addItem("All covers"); refreshBtn=new QPushButton("⟳ Refresh"); refreshBtn->setObjectName("BackBtn"); auto *csv=new QPushButton("CSV ↓"); csv->setObjectName("GreenPrimaryBtn"); toolbarRow->addWidget(search,2); toolbarRow->addWidget(status); toolbarRow->addWidget(cover); toolbarRow->addStretch(); toolbarRow->addWidget(refreshBtn); toolbarRow->addWidget(csv); recordsBox->addWidget(toolbar);
    auto *recordsRow=new QHBoxLayout;
    recordsTable=new QTableWidget; recordsTable->setColumnCount(8); recordsTable->setHorizontalHeaderLabels({"Policy ID","Client","Phone","Cover","Risk score","Monthly (M)","Annual (M)","Status"}); recordsTable->setAlternatingRowColors(true); recordsTable->setSelectionBehavior(QAbstractItemView::SelectRows); recordsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); recordsRow->addWidget(recordsTable,4);
    auto *drawer=card(); auto *drawerBox=new QVBoxLayout(drawer); drawerBox->setContentsMargins(20,20,20,20); drawerBox->addWidget(text("Policy details","title"));
    auto *detailText=text("Select a policy record to view client, billing, and decision details.","muted"); detailText->setWordWrap(true); drawerBox->addWidget(detailText); drawerBox->addSpacing(14); drawerBox->addWidget(text("Billing schedule","title"));
    auto *billingText=text("No policy selected","muted"); billingText->setWordWrap(true); drawerBox->addWidget(billingText); drawerBox->addStretch();
    auto *statement=new QPushButton("Billing statement (PDF)"); statement->setEnabled(false); drawerBox->addWidget(statement);
    auto *change=new QPushButton("Change status"); change->setEnabled(false); drawerBox->addWidget(change);
    recordsRow->addWidget(drawer,1); recordsBox->addLayout(recordsRow,1); recordsBox->addWidget(text("Database connected - insurerisk.db", "muted")); tabWidget->addTab(recordsTab,"Policy Records");
// Analytics
    auto *analyticsTab=new QWidget; auto *analyticsBox=new QVBoxLayout(analyticsTab); analyticsBox->setContentsMargins(0,10,0,8); auto *analyticsKpis=new QHBoxLayout; for(QWidget *w:{kpi("Applications","0","No records loaded"),kpi("Acceptance rate","—","Refresh portfolio"),kpi("Avg risk score","—","Accepted policies"),kpi("Annual premium revenue","M —","Accepted policies", "#16a34a"),kpi("Avg VaR 95%","M —","Capital held", "#dc2626")}) analyticsKpis->addWidget(w); analyticsBox->addLayout(analyticsKpis); auto *chartRow=new QHBoxLayout; auto *outcomes=card(); auto *outcomesBox=new QVBoxLayout(outcomes); outcomesBox->setContentsMargins(20,18,20,18); outcomesBox->addWidget(text("Underwriting outcomes","title")); QChart *chart=new QChart; chart->legend()->setAlignment(Qt::AlignBottom); chart->setBackgroundVisible(false); chartView=new QChartView(chart); chartView->setRenderHint(QPainter::Antialiasing); outcomesBox->addWidget(chartView,1); chartRow->addWidget(outcomes,1); auto *premium=card(); auto *pl=new QVBoxLayout(premium); pl->setContentsMargins(20,18,20,18); pl->addWidget(text("Annual premium by cover (M)","title")); for(int i=0;i<4;++i){auto *bar=new QProgressBar;bar->setObjectName("coverBar"+QString::number(i));bar->setRange(0,100);bar->setValue(0);bar->setFormat("M 0.00");pl->addWidget(bar);} pl->addStretch(); chartRow->addWidget(premium,1); auto *tiersCard=card(); auto *rtl=new QVBoxLayout(tiersCard); rtl->setContentsMargins(20,18,20,18); rtl->addWidget(text("Risk tier distribution","title")); for(int i=0;i<4;++i){auto *bar=new QProgressBar;bar->setObjectName("tierBar"+QString::number(i));bar->setRange(0,100);bar->setValue(0);bar->setFormat("0 policies");rtl->addWidget(bar);} rtl->addStretch(); chartRow->addWidget(tiersCard,1); analyticsBox->addLayout(chartRow,1); auto *bottom=new QHBoxLayout; auto *recent=card(); auto *recentBox=new QVBoxLayout(recent); recentBox->setContentsMargins(20,18,20,18); recentBox->addWidget(text("Recent applications","title")); analyticsSummaryLabel=text("Refresh analytics to load the latest portfolio metrics.","muted"); recentBox->addWidget(analyticsSummaryLabel); recentBox->addStretch(); bottom->addWidget(recent,2); auto *varCard=card(); auto *varBox=new QVBoxLayout(varCard); varBox->setContentsMargins(20,18,20,18); varBox->addWidget(text("Value-at-Risk 95% by cover (M)","title")); varBox->addWidget(text("Property  ▰▰▰▰▰  27,400\n\nMotor       ▰▰▰▰    24,100\n\nLife          ▰▰▰       16,800\n\nHealth      ▰▰          11,900")); bottom->addWidget(varCard,1); analyticsBox->addLayout(bottom,1); auto *refreshAnalyticsBtn=new QPushButton("Refresh analytics"); refreshAnalyticsBtn->setObjectName("GreenPrimaryBtn"); analyticsBox->addWidget(refreshAnalyticsBtn); tabWidget->addTab(analyticsTab,"Stress Testing");


    // Home: a focused launcher, not a dashboard full of widgets.
    auto *homePage = new QWidget;
    auto *homeLayout = new QVBoxLayout(homePage);
    homeLayout->setContentsMargins(64, 46, 64, 46);
    homeLayout->setSpacing(22);
    auto *hero = new QFrame; hero->setObjectName("homeHero");
    auto *heroLayout = new QVBoxLayout(hero); heroLayout->setContentsMargins(30, 24, 30, 24);
    auto *heroTitle = new QLabel("Insurance decisions made clear"); heroTitle->setStyleSheet("font-size:28px;font-weight:700;color:#ffffff;");
    auto *heroText = new QLabel("Start with one simple task. Your information and quotations are saved automatically."); heroText->setStyleSheet("font-size:14px;color:#dcfce7;");
    heroLayout->addWidget(heroTitle); heroLayout->addWidget(heroText);
    homeLayout->addWidget(hero);
    homeLayout->addWidget(text("What would you like to do?", "title"));

    auto *serviceGrid = new QGridLayout;
    serviceGrid->setHorizontalSpacing(20); serviceGrid->setVerticalSpacing(20);
    auto makeService = [&text](const QString &number, const QString &title, const QString &description) {
        auto *frame = new QFrame; frame->setObjectName("homeService"); frame->setMinimumSize(330, 190);
        auto *layout = new QVBoxLayout(frame); layout->setContentsMargins(22, 20, 22, 20); layout->setSpacing(9);
        auto *top = new QHBoxLayout; auto *icon = text(number); icon->setObjectName("homeIcon"); icon->setAlignment(Qt::AlignCenter); icon->setFixedSize(46,46);
        top->addWidget(icon); top->addStretch(); layout->addLayout(top);
        auto *heading = text(title); heading->setObjectName("homeCardTitle"); layout->addWidget(heading);
        auto *body = text(description, "muted"); body->setWordWrap(true); layout->addWidget(body); layout->addStretch();
        auto *action = new QPushButton("Open service"); action->setObjectName("homeAction"); layout->addWidget(action, 0, Qt::AlignLeft);
        return qMakePair(frame, action);
    };
    const auto dashboardService = makeService("01", "Risk assessment", "Assess a client's risk, generate a premium, and make a decision.");
    const auto pricingService = makeService("02", "Pricing and quotation", "Compare straightforward premium and deductible options.");
    const auto recordsService = makeService("03", "Policy records", "Find saved policies, create a billing statement, or update a status.");
    const auto portfolioService = makeService("04", "Portfolio overview", "Review portfolio trends and advisory information.");
    serviceGrid->addWidget(dashboardService.first, 0, 0);
    serviceGrid->addWidget(pricingService.first, 0, 1);
    serviceGrid->addWidget(recordsService.first, 1, 0);
    serviceGrid->addWidget(portfolioService.first, 1, 1);
    homeLayout->addLayout(serviceGrid);
    homeLayout->addStretch();

    workspaceStack->addWidget(homePage);
    workspaceStack->addWidget(tabWidget);
    workspaceStack->setCurrentWidget(homePage);
    const auto openService = [workspaceStack, this](const QString &tabName) {
        for (int i = 0; i < tabWidget->count(); ++i)
            if (tabWidget->tabText(i) == tabName) { tabWidget->setCurrentIndex(i); break; }
        workspaceStack->setCurrentWidget(tabWidget);
    };
    connect(homeBtn, &QPushButton::clicked, this, [workspaceStack, homePage](){ workspaceStack->setCurrentWidget(homePage); });
    connect(dashboardService.second, &QPushButton::clicked, this, [openService](){ openService("Dashboard"); });
    connect(pricingService.second, &QPushButton::clicked, this, [openService](){ openService("Pricing & Quotation"); });
    connect(recordsService.second, &QPushButton::clicked, this, [openService](){ openService("Policy Records"); });
    connect(portfolioService.second, &QPushButton::clicked, this, [openService](){ openService("Stress Testing"); });
    connect(showPassword,&QCheckBox::toggled,this,[this](bool on){passLoginInput->setEchoMode(on?QLineEdit::Normal:QLineEdit::Password);});
    connect(loginBtn,&QPushButton::clicked,this,&MainWindow::handleLogin); connect(registerBtn,&QPushButton::clicked,this,&MainWindow::handleRegister); connect(logoutBtn,&QPushButton::clicked,this,&MainWindow::handleLogout); connect(calcBtn,&QPushButton::clicked,this,&MainWindow::runSimulation); connect(acceptBtn,&QPushButton::clicked,this,&MainWindow::acceptApplication); connect(rejectBtn,&QPushButton::clicked,this,&MainWindow::rejectApplication); connect(refreshBtn,&QPushButton::clicked,this,&MainWindow::refreshDatabaseTable);
    connect(clearBtn,&QPushButton::clicked,this,[this]{ for (auto *e : {policyIdInput,clientNameInput,phoneInput,addressInput,ageInput,incomeInput,loansInput,assetInput,riskParamInput}) e->clear(); resultsOutput->clear(); simProgressBar->setValue(0); });

    connect(recordsTable,&QTableWidget::cellClicked,this,[this,detailText,billingText,statement,change](int row,int){
        const auto cell=[this,row](int col){auto *i=recordsTable->item(row,col);return i?i->text():QString();};
        detailText->setText("<b>"+cell(0)+" � "+cell(1)+"</b><br>Phone: "+cell(2)+"<br>Cover: "+cell(3)+"<br>Risk score: "+cell(4)+"<br>Status: "+cell(7));
        billingText->setText("Monthly premium: M "+cell(5)+"<br>Annual premium: M "+cell(6)+"<br>Next billing date: the first day of the next month.");
        statement->setEnabled(true); change->setEnabled(true);
    });
    connect(change,&QPushButton::clicked,this,[this,analyticsTab](){
        const int row=recordsTable->currentRow(); if(row<0) return; auto *idItem=recordsTable->item(row,0); if(!idItem) return;
        const QString current=recordsTable->item(row,7)->text();
        const QString next=QInputDialog::getItem(this,"Change policy status","Status",{"Approved","Rejected"},current=="Rejected"?1:0,false);
        if(next.isEmpty()) return; const int dbId=idItem->data(Qt::UserRole).toInt();
        if(DatabaseManager::updatePolicyStatus(dbId,next.toStdString())) { refreshDatabaseTable(); QMessageBox::information(this,"Status updated","Policy status changed to "+next+"."); }
        else QMessageBox::warning(this,"Status update","The policy status could not be changed.");
    });
    connect(statement,&QPushButton::clicked,this,[this,analyticsTab](){
        const int row=recordsTable->currentRow(); if(row<0) return;
        const auto cell=[this,row](int col){auto *i=recordsTable->item(row,col);return i?i->text():QString();};
        const QString file=QFileDialog::getSaveFileName(this,"Save billing statement",cell(0)+"-statement.pdf","PDF files (*.pdf)"); if(file.isEmpty())return;
        QPdfWriter writer(file); writer.setResolution(72); writer.setPageSize(QPageSize(QPageSize::A4)); QPainter painter(&writer); const int w=writer.width(),m=120;
        painter.fillRect(0,0,w,180,QColor("#16a34a")); painter.setPen(Qt::white); painter.setFont(QFont("Segoe UI",18,QFont::Bold)); painter.drawText(m,90,"Matseliso_InsureRisk");
        painter.setFont(QFont("Segoe UI",11)); painter.drawText(m,135,"Billing statement"); painter.setPen(QColor("#1b2430")); painter.setFont(QFont("Segoe UI",11));
        int y=270; const QList<QPair<QString,QString>> rows={{"Policy ID",cell(0)},{"Client",cell(1)},{"Cover",cell(3)},{"Policy status",cell(7)},{"Monthly premium","M "+cell(5)},{"Annual premium","M "+cell(6)}};
        for(const auto &entry:rows){painter.setPen(QColor("#d9e1ea"));painter.drawLine(m,y-22,w-m,y-22);painter.setPen(QColor("#1b2430"));painter.drawText(m,y,entry.first);painter.drawText(m+380,y,entry.second);y+=52;} painter.end(); QMessageBox::information(this,"Statement created","The billing statement was saved.");
    });
    connect(recordsTable,&QTableWidget::cellChanged,this,[this](int row,int){ auto *state=recordsTable->item(row,7); if(state && state->text()=="Rejected"){ QMessageBox::warning(this,"Restriction","Rejected applications are read-only."); refreshDatabaseTable(); }});
    connect(refreshAnalyticsBtn,&QPushButton::clicked,this,[this,analyticsTab](){ sqlite3 *db=nullptr; int accepted=0,rejected=0; if(sqlite3_open("insurerisk.db",&db)==SQLITE_OK){ sqlite3_stmt *stmt=nullptr; if(sqlite3_prepare_v2(db,"SELECT status, COUNT(*) FROM policies GROUP BY status;",-1,&stmt,nullptr)==SQLITE_OK){while(sqlite3_step(stmt)==SQLITE_ROW){QString st=QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt,0))); if(st=="Accepted" || st=="Approved") accepted+=sqlite3_column_int(stmt,1); if(st=="Rejected") rejected=sqlite3_column_int(stmt,1);} sqlite3_finalize(stmt);} sqlite3_close(db);} QVector<double> livePremiums(4,0.0); QVector<int> liveTiers(4,0); if(sqlite3_open("insurerisk.db",&db)==SQLITE_OK){sqlite3_stmt *detail=nullptr;if(sqlite3_prepare_v2(db,"SELECT cover_type,annual_premium,risk_score FROM policies;",-1,&detail,nullptr)==SQLITE_OK){while(sqlite3_step(detail)==SQLITE_ROW){const QString c=QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(detail,0)));int ci=c.contains("Life")?0:c.contains("Car")?1:c.contains("Property")?2:3;livePremiums[ci]+=sqlite3_column_double(detail,1);const double score=sqlite3_column_double(detail,2);liveTiers[score<30?0:score<55?1:score<75?2:3]++;}sqlite3_finalize(detail);}sqlite3_close(db);}double highPremium=1;for(double p:livePremiums)if(p>highPremium)highPremium=p;int highCount=1;for(int n:liveTiers)if(n>highCount)highCount=n;for(int i=0;i<4;++i){if(auto *bar=analyticsTab->findChild<QProgressBar*>("coverBar"+QString::number(i))){bar->setValue(int(livePremiums[i]*100/highPremium));bar->setFormat("M "+QString::number(livePremiums[i],'f',2));}if(auto *bar=analyticsTab->findChild<QProgressBar*>("tierBar"+QString::number(i))){bar->setValue(liveTiers[i]*100/highCount);bar->setFormat(QString::number(liveTiers[i])+" policies");}} auto *series=new QPieSeries; series->setHoleSize(.55); if(accepted) {auto *s=series->append("Accepted "+QString::number(accepted),accepted);s->setColor(QColor("#16a34a"));} if(rejected) {auto *s=series->append("Rejected "+QString::number(rejected),rejected);s->setColor(QColor("#dc2626"));} if(!accepted&&!rejected){auto *s=series->append("No applications",1);s->setColor(QColor("#d9e1ea"));} auto *next=new QChart; next->addSeries(series); next->setBackgroundVisible(false); next->legend()->setAlignment(Qt::AlignBottom); chartView->setChart(next); analyticsSummaryLabel->setText(QString("Portfolio snapshot · %1 accepted · %2 rejected").arg(accepted).arg(rejected)); });

    const QString lightStyle = styleSheet();
    const auto applyTheme = [this, themeBtn, lightStyle](bool dark) { themeBtn->setText(dark ? "Light mode" : "Dark mode"); setStyleSheet(dark ? lightStyle + " QMainWindow,QWidget{background:#0b1220;color:#e6ebf5;} QLabel{color:#e6ebf5;} QFrame#card,QFrame#kpi{background:#111a2e;border-color:#26334f;} QFrame#homeService{background:#111a2e;border-color:#26334f;} QLabel#homeIcon{background:#14532d;color:#dcfce7;} QPushButton#serviceCard{background:#111a2e;color:#e6ebf5;border-color:#26334f;} QPushButton#serviceCard:hover,QPushButton#serviceCard:focus{background:#14532d;color:#ffffff;border-color:#16a34a;} QLineEdit,QComboBox,QTextEdit,QTableWidget{background:#0d1627;color:#e6ebf5;border-color:#26334f;} QTableWidget::item{color:#e6ebf5;background:#0d1627;} QTableWidget::item:selected{color:#e6ebf5;background:#1e3a2f;} QHeaderView::section{background:#17223a;color:#e6ebf5;} QTabWidget::pane{background:#0b1220;} QLabel#modelRow{background:#17223a;color:#e6ebf5;} QProgressBar{background:#26334f;color:#e6ebf5;}" : lightStyle); };
    darkTheme = DatabaseManager::getSetting("theme", "light") == "dark";
    applyTheme(darkTheme);
    connect(themeBtn,&QPushButton::clicked,this,[this, applyTheme](){ darkTheme=!darkTheme; DatabaseManager::setSetting("theme", darkTheme ? "dark" : "light"); applyTheme(darkTheme); });
    connect(csv,&QPushButton::clicked,this,[this,analyticsTab](){const QString file=QFileDialog::getSaveFileName(this,"Export policy records",QString(),"CSV files (*.csv)");if(file.isEmpty())return;QSaveFile out(file);if(!out.open(QIODevice::WriteOnly|QIODevice::Text)){QMessageBox::warning(this,"Export failed","Unable to write the selected file.");return;}QTextStream stream(&out);for(int c=0;c<recordsTable->columnCount();++c){if(c)stream<<",";stream<<recordsTable->horizontalHeaderItem(c)->text();}stream<<"\n";for(int r=0;r<recordsTable->rowCount();++r){for(int c=0;c<recordsTable->columnCount();++c){if(c)stream<<",";auto *item=recordsTable->item(r,c);stream<<(item?item->text():QString());}stream<<"\n";}if(out.commit())QMessageBox::information(this,"CSV export","Policy records exported successfully.");});
    connect(quoteBtn,&QPushButton::clicked,this,[this,analyticsTab](){
        if(latestReport.isEmpty()) return;
        const QString file=QFileDialog::getSaveFileName(this,"Save actuarial quote",QString("actuarial-quote.pdf"),"PDF files (*.pdf)");
        if(file.isEmpty()) return;
        QPdfWriter writer(file); writer.setResolution(72); writer.setPageSize(QPageSize(QPageSize::A4)); writer.setPageMargins(QMarginsF(15,15,15,15));
        QPainter painter(&writer); const int w=writer.width(), margin=120, y0=150;
        painter.fillRect(0,0,w,180,QColor("#16a34a")); painter.setPen(Qt::white); painter.setFont(QFont("Segoe UI",18,QFont::Bold));
        painter.drawText(QRect(margin,45,w-margin*2,60),Qt::AlignLeft|Qt::AlignVCenter,"Matseliso_InsureRisk");
        painter.setFont(QFont("Segoe UI",11)); painter.drawText(QRect(margin,105,w-margin*2,35),Qt::AlignLeft,"Actuarial Quote");
        painter.setPen(QColor("#1b2430")); painter.setFont(QFont("Segoe UI",11)); int y=y0+70;
        painter.drawText(margin,y,"Policy ID"); painter.drawText(margin+260,y,policyIdInput->text()); y+=42;
        painter.drawText(margin,y,"Client"); painter.drawText(margin+260,y,clientNameInput->text()); y+=55;
        const QList<QPair<QString,QString>> rows={{"Expected annual loss","M "+QString::number(latestExpectedLoss,'f',2)},{"Risk-loaded loss","M "+QString::number(latestRiskLoadedLoss,'f',2)},{"Capital charge","M "+QString::number(latestCapitalCharge,'f',2)},{"Administration fee","M 150.00"}};
        painter.fillRect(margin,y,w-margin*2,34,QColor("#f3f6fa")); painter.setFont(QFont("Segoe UI",11,QFont::Bold)); painter.drawText(margin+12,y+23,"Pricing item"); painter.drawText(QRect(margin+280,y,w-margin-(margin+280),34),Qt::AlignRight|Qt::AlignVCenter,"Amount"); y+=34;
        painter.setFont(QFont("Segoe UI",11));
        for(const auto &row:rows){ painter.setPen(QColor("#d9e1ea")); painter.drawLine(margin,y,w-margin,y); painter.setPen(QColor("#1b2430")); painter.drawText(margin+12,y+28,row.first); painter.drawText(QRect(margin+280,y,w-margin-(margin+280),42),Qt::AlignRight|Qt::AlignVCenter,row.second); y+=42; }
        painter.setPen(QColor("#16a34a")); painter.fillRect(margin,y,w-margin*2,52,QColor("#ecfdf3")); painter.setFont(QFont("Segoe UI",12,QFont::Bold)); painter.drawText(margin+12,y+32,"Annual premium"); painter.drawText(w-margin-160,y+32,"M "+QString::number(latestAnnualPremium,'f',2)); y+=72;
        painter.fillRect(margin,y,w-margin*2,52,QColor("#ecfdf3")); painter.setPen(QColor("#15803d")); painter.drawText(margin+12,y+32,"Monthly premium"); painter.drawText(w-margin-160,y+32,"M "+QString::number(latestMonthlyPremium,'f',2)); y+=86;
        painter.setPen(QColor("#1b2430")); painter.setFont(QFont("Segoe UI",11,QFont::Bold)); painter.drawText(margin,y,"Underwriting summary"); y+=27; painter.setFont(QFont("Segoe UI",10)); painter.drawText(QRect(margin,y,w-margin*2,80),Qt::TextWordWrap,"This quote is based on the policy details supplied. The assessed risk score is "+QString::number(latestScore,'f',2)+". Please contact your underwriter if you need help understanding your cover or premium.");
        painter.end(); QMessageBox::information(this,"Quote created","Your structured actuarial quote was saved.");
    });

}
void MainWindow::handleLogin(){
    const std::string user=userLoginInput->text().toStdString(), pass=passLoginInput->text().toStdString();
    if(user.empty()||pass.empty()){QMessageBox::warning(this,"Error","Username and password cannot be empty.");return;}
    if(AuthManager::loginUser(user,pass)){currentUser=QString::fromStdString(user);authStatusLabel->setText("● Signed in as "+currentUser);centralStack->setCurrentIndex(1);refreshDatabaseTable();QMessageBox::information(this,"Success","Welcome back, "+currentUser+"!");}
    else QMessageBox::warning(this,"Login Failed","Invalid username or password. Please register first.");
}
void MainWindow::handleRegister(){
    const std::string user=userLoginInput->text().toStdString(), pass=passLoginInput->text().toStdString();
    if(user.empty()||pass.empty()){QMessageBox::warning(this,"Error","Username and password cannot be empty.");return;}
    if(AuthManager::registerUser(user,pass)) QMessageBox::information(this,"Success","Account registered successfully! You can now log in.");
    else QMessageBox::warning(this,"Error","Username already exists or database error.");
}
void MainWindow::handleLogout(){currentUser.clear();userLoginInput->clear();passLoginInput->clear();authStatusLabel->setText("● Ready\nPlease log in or register to continue.");centralStack->setCurrentIndex(0);}

void MainWindow::runSimulation(){
    bool okAge,okIncome,okLoans,okAsset,okRisk; const std::string id=policyIdInput->text().toStdString(); std::string name=clientNameInput->text().toStdString(); if(name.empty()) name="Thabo Mafokane";
    const int age=ageInput->text().toInt(&okAge), risk=riskParamInput->text().toInt(&okRisk); const double income=incomeInput->text().toDouble(&okIncome),loans=loansInput->text().toDouble(&okLoans),asset=assetInput->text().toDouble(&okAsset);
    if(!okAge||!okIncome||!okLoans||!okAsset||!okRisk||id.empty()||age<18||asset<=0){QMessageBox::warning(this,"Validation Error","Please fill in all fields with valid client details and numeric parameters.");return;}
    simProgressBar->setValue(25);QCoreApplication::processEvents();
    Policyholder ph(id,currentUser.isEmpty()?"Admin":currentUser.toStdString(),name,phoneInput->text().toStdString(),addressInput->text().toStdString(),static_cast<CoverType>(coverTypeCombo->currentIndex()),age,income,loans,geoZoneCombo->currentText().toStdString(),asset,risk);
    simProgressBar->setValue(60);QCoreApplication::processEvents(); double score=RiskEvaluator::calculateRiskScore(ph), loss=RiskEvaluator::calculateExpectedLoss(ph); MonteCarloSimulator sim(10000,4); const double var=sim.calculateVaR(sim.simulateLosses(ph),.95); ActuarialEngine engine(.15,.05); const double annual=engine.calculateRiskAdjustedPremium(ph,var), monthly=annual/12.; simProgressBar->setValue(100);
    riskScoreKpi->findChild<QLabel*>("value");
    const auto values=[&](QFrame *card,const QString &v){for(auto *label:card->findChildren<QLabel*>())if(label->objectName()=="value"){label->setText(v);break;}}; values(riskScoreKpi,QString::number(score,'f',2));values(expectedLossKpi,"M "+QString::number(loss,'f',2));values(varKpi,"M "+QString::number(var,'f',2));values(monthlyPremiumKpi,"M "+QString::number(monthly,'f',2));
    const bool decline=score>=75, refer=score>=55; decisionBadge->setText(decline?"● RECOMMEND DECLINE":refer?"● REFER TO SENIOR UNDERWRITER":"● RECOMMEND ACCEPT");
    latestScore=score; latestExpectedLoss=loss; latestRiskLoadedLoss=loss*1.25; latestCapitalCharge=(var-loss)*.06; latestAnnualPremium=annual; latestMonthlyPremium=monthly;
    resultsOutput->setPlainText(QString("Expected annual loss                         M %1\nRisk-loaded loss (× 1.25)                    M %2\nCapital charge                                M %3\nAdmin fee                                     M 150.00\n\nANNUAL SUBTOTAL                               M %4\n\nMONTHLY PREMIUM                               M %5\n\nUnderwriting notes\n• Risk assessment completed using your policy details.\n• Final decision follows the published risk thresholds.").arg(loss,0,'f',2).arg(loss*1.25,0,'f',2).arg((var-loss)*.06,0,'f',2).arg(annual,0,'f',2).arg(monthly,0,'f',2).arg(score,0,'f',2));
    const int zonePoints=geoZoneCombo->currentText()=="Urban"?5:(geoZoneCombo->currentText()=="Suburban"?9:15); const int driverValues[5]={qBound(0,age*25/80,25),qBound(0,static_cast<int>((loans/qMax(1.0,income*12.0))*25.0),25),zonePoints,qBound(0,risk*5,25),qBound(0,static_cast<int>(asset/50000.0*10.0),10)}; for(int i=0;i<riskDriverBars.size()&&i<5;++i) riskDriverBars[i]->setValue(driverValues[i]); latestReport=resultsOutput->toPlainText(); acceptBtn->setEnabled(true);rejectBtn->setEnabled(true);referBtn->setEnabled(true);quoteBtn->setEnabled(true);
}

static bool saveCurrentPolicy(MainWindow *w,const QString &status){
    Q_UNUSED(w); return true;
}
void MainWindow::acceptApplication(){
    const std::string id=policyIdInput->text().toStdString(); if(id.empty()){QMessageBox::warning(this,"Error","Policy ID cannot be empty.");return;} std::string name=clientNameInput->text().toStdString();if(name.empty())name="Thabo Mafokane";
    Policyholder ph(id,currentUser.isEmpty()?"Admin":currentUser.toStdString(),name,phoneInput->text().toStdString(),addressInput->text().toStdString(),static_cast<CoverType>(coverTypeCombo->currentIndex()),ageInput->text().toInt(),incomeInput->text().toDouble(),loansInput->text().toDouble(),geoZoneCombo->currentText().toStdString(),assetInput->text().toDouble(),riskParamInput->text().toInt());
    double score=RiskEvaluator::calculateRiskScore(ph),loss=RiskEvaluator::calculateExpectedLoss(ph);MonteCarloSimulator sim(10000,4);double var=sim.calculateVaR(sim.simulateLosses(ph),.95);ActuarialEngine eng(.15,.05); if(DatabaseManager::savePolicyRecord(ph,score,loss,var,eng.calculateRiskAdjustedPremium(ph,var),"Approved")){QMessageBox::information(this,"Success","Application approved and saved.");refreshDatabaseTable();acceptBtn->setEnabled(false);rejectBtn->setEnabled(false);simProgressBar->setValue(0);}else QMessageBox::critical(this,"Database Error","Failed to save policy record to SQLite database.");
}
void MainWindow::rejectApplication(){
    const std::string id=policyIdInput->text().toStdString(); if(id.empty()){QMessageBox::warning(this,"Error","Policy ID cannot be empty.");return;} std::string name=clientNameInput->text().toStdString();if(name.empty())name="Thabo Mafokane";
    Policyholder ph(id,currentUser.isEmpty()?"Admin":currentUser.toStdString(),name,phoneInput->text().toStdString(),addressInput->text().toStdString(),static_cast<CoverType>(coverTypeCombo->currentIndex()),ageInput->text().toInt(),incomeInput->text().toDouble(),loansInput->text().toDouble(),geoZoneCombo->currentText().toStdString(),assetInput->text().toDouble(),riskParamInput->text().toInt());
    double score=RiskEvaluator::calculateRiskScore(ph),loss=RiskEvaluator::calculateExpectedLoss(ph);MonteCarloSimulator sim(10000,4);double var=sim.calculateVaR(sim.simulateLosses(ph),.95);ActuarialEngine eng(.15,.05); if(DatabaseManager::savePolicyRecord(ph,score,loss,var,eng.calculateRiskAdjustedPremium(ph,var),"Rejected")){QMessageBox::information(this,"Application Rejected","Application has been archived as rejected.");refreshDatabaseTable();acceptBtn->setEnabled(false);rejectBtn->setEnabled(false);simProgressBar->setValue(0);}else QMessageBox::critical(this,"Database Error","Failed to save rejected policy record to SQLite database.");
}
void MainWindow::refreshDatabaseTable(){
    sqlite3 *db=nullptr; if(sqlite3_open("insurerisk.db",&db)!=SQLITE_OK){if(db)sqlite3_close(db);return;}
    sqlite3_stmt *stmt=nullptr; const char *query="SELECT id, policy_id, client_name, phone_number, cover_type, risk_score, monthly_premium, annual_premium, status FROM policies ORDER BY id DESC;";
    if(sqlite3_prepare_v2(db,query,-1,&stmt,nullptr)!=SQLITE_OK){sqlite3_close(db);return;}
    recordsTable->blockSignals(true); recordsTable->setRowCount(0); int row=0;
    while(sqlite3_step(stmt)==SQLITE_ROW){
        recordsTable->insertRow(row);
        for(int col=0;col<8;++col){
            const int source=col+1; QString value;
            if(col==4) value=QString::number(sqlite3_column_double(stmt,source),'f',2);
            else if(col==5 || col==6) value=QString::number(sqlite3_column_double(stmt,source),'f',2);
            else {const auto *raw=sqlite3_column_text(stmt,source);value=raw?QString::fromUtf8(reinterpret_cast<const char*>(raw)):QString();}
            if(col==7 && value=="Accepted") value="Approved";
            auto *item=new QTableWidgetItem(value);
            if(col==0) item->setData(Qt::UserRole,sqlite3_column_int(stmt,0));
            if(col==7){if(value=="Approved") item->setForeground(QColor("#16a34a")); else if(value=="Rejected") item->setForeground(QColor("#dc2626")); item->setTextAlignment(Qt::AlignCenter);}
            recordsTable->setItem(row,col,item);
        } ++row;
    }
    recordsTable->blockSignals(false); sqlite3_finalize(stmt); sqlite3_close(db);
}












































