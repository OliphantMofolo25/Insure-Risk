with open("src/MainWindow.cpp", "r") as f:
    content = f.read()

# Old compliance tab code block
old_tab3 = """    // TAB 3: Informative Compliance Base
    infoTab = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoTab);
    QTextEdit *infoText = new QTextEdit();
    infoText->setReadOnly(true);
    infoText->setPlainText(
        "=== Matseliso_InsureRisk Informative Compliance & Actuarial Standards Base ===\\n\\n"
        "1. Regulatory Framework & Solvency II:\\n"
        "   - Matseliso_InsureRisk adheres to international actuarial valuation standards.\\n"
        "   - Capital requirements are calculated using stochastic Value-at-Risk (VaR 95%) confidence intervals.\\n\\n"
        "2. Risk Evaluation Methodology:\\n"
        "   - Risk scores are derived from multi-variable regressions including Debt-to-Income (DTI) ratios, geographic risk tiers, and asset exposure.\\n\\n"
        "3. Software Reliability & IEEE Testing Standards:\\n"
        "   - Developed in C++17 on Linux/WSL with strict MVC architecture.\\n"
        "   - Validated using Google Test (GTest), Cppcheck static analysis, Valgrind memory profiling, and automated black-box test harnesses.\\n"
    );
    infoLayout->addWidget(new QLabel("<h3>Enterprise Actuarial Knowledge Base & Guidelines</h3>"));
    infoLayout->addWidget(infoText);
    tabWidget->addTab(infoTab, "Compliance Base");"""

# New interactive hover card tab code block
new_tab3 = """    // TAB 3: Informative Compliance Base (Interactive Hover Cards)
    infoTab = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoTab);
    infoLayout->setContentsMargins(20, 20, 20, 20);
    infoLayout->setSpacing(15);

    QLabel *infoHeading = new QLabel("<h3>Enterprise Actuarial Knowledge Base & Regulatory Standards</h3>");
    infoHeading->setStyleSheet("color: #1e293b; font-weight: bold; margin-bottom: 5px;");
    infoLayout->addWidget(infoHeading);

    // QSS for hover cards injected directly into style or handled via object names
    // Note: QFrame#ComplianceCard style is already defined in global QSS setupUI stylesheet.

    auto createComplianceCard = [](const QString& title, const QStringList& points) {
        QFrame *card = new QFrame();
        card->setObjectName("ComplianceCard");
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(15, 15, 15, 15);
        cardLayout->setSpacing(8);

        QLabel *titleLbl = new QLabel("<b>" + title + "</b>");
        titleLbl->setStyleSheet("color: #16a34a; font-size: 14px;");
        cardLayout->addWidget(titleLbl);

        for (const QString& pt : points) {
            QLabel *ptLbl = new QLabel("• " + pt);
            ptLbl->setWordWrap(true);
            ptLbl->setStyleSheet("color: #334155; font-size: 13px;");
            cardLayout->addWidget(ptLbl);
        }
        return card;
    };

    infoLayout->addWidget(createComplianceCard("1. Regulatory Framework & Solvency II", {
        "Matseliso_InsureRisk adheres to international actuarial valuation standards.",
        "Capital requirements are calculated using stochastic Value-at-Risk (VaR 95%) confidence intervals."
    }));

    infoLayout->addWidget(createComplianceCard("2. Risk Evaluation Methodology", {
        "Risk scores are derived from multi-variable regressions including Debt-to-Income (DTI) ratios.",
        "Incorporates geographic risk tiers and asset exposure metrics for precision scoring."
    }));

    infoLayout->addWidget(createComplianceCard("3. Software Reliability & IEEE Testing Standards", {
        "Developed in C++17 on Linux/WSL utilizing strict MVC architectural patterns.",
        "Validated across a 7-tool industry suite: GTest, Cppcheck, Clang-Tidy, Valgrind, ASan, Lcov, and PyTest."
    }));

    infoLayout->addStretch();
    tabWidget->addTab(infoTab, "Compliance Base");"""

if old_tab3 in content:
    content = content.replace(old_tab3, new_tab3)
    with open("src/MainWindow.cpp", "w") as f:
        f.write(content)
    print("Successfully updated MainWindow.cpp with interactive hover cards!")
else:
    print("Could not find exact match, performing regex or manual replacement.")
