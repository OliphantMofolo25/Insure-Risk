#include "MainWindow.hpp"
#include "LearningCentre.hpp"
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
#include <QDate>
#include <QIcon>
#include <QKeySequence>
#include <QLocale>
#include <QStyle>
#include <QButtonGroup>
#include <array>
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

namespace {
enum class InsuranceIcon { Shield, Quote, Policy, Portfolio, Motor, Property };

QIcon insuranceIcon(InsuranceIcon icon) {
    QPixmap image(72, 72);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(3, 3);
    painter.setPen(QPen(QColor("#166534"), 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    switch (icon) {
    case InsuranceIcon::Shield:
        painter.drawPolygon(QPolygonF{{12,3},{20,6},{19,14},{16,18},{12,21},{8,18},{5,14},{4,6}});
        painter.drawPolyline(QPolygonF{{8,12},{11,15},{16,9}});
        break;
    case InsuranceIcon::Quote:
        painter.drawRoundedRect(QRectF(5,3,14,18), 2, 2);
        painter.drawLine(8, 8, 16, 8);
        painter.drawLine(8, 12, 16, 12);
        painter.drawLine(8, 16, 13, 16);
        break;
    case InsuranceIcon::Policy:
        painter.drawPolygon(QPolygonF{{3,6},{10,6},{12,8},{21,8},{21,19},{3,19}});
        painter.drawLine(7, 12, 17, 12);
        painter.drawLine(7, 15, 14, 15);
        break;
    case InsuranceIcon::Portfolio:
        painter.drawPolyline(QPolygonF{{4,4},{4,20},{21,20}});
        painter.drawLine(8, 16, 8, 12);
        painter.drawLine(13, 16, 13, 9);
        painter.drawLine(18, 16, 18, 5);
        break;
    case InsuranceIcon::Motor:
        painter.drawPolyline(QPolygonF{{4,11},{7,6},{17,6},{20,11}});
        painter.drawRoundedRect(QRectF(3,11,18,6), 1.5, 1.5);
        painter.drawLine(6, 17, 6, 20);
        painter.drawLine(18, 17, 18, 20);
        break;
    case InsuranceIcon::Property:
        painter.drawPolyline(QPolygonF{{3,11},{12,3},{21,11}});
        painter.drawRect(QRectF(6,10,12,11));
        painter.drawRect(QRectF(10,15,4,6));
        break;
    }
    painter.end();
    image.setDevicePixelRatio(3);
    return QIcon(image);
}

QPixmap insuranceBanner(QSize size = QSize(224, 80)) {
    QPixmap image(size * 2);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(image.width() / 224.0, image.height() / 80.0);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(220, 252, 231, 35));
    painter.drawRoundedRect(QRectF(14, 8, 202, 64), 32, 32);

    const QPen outline(QColor("#bbf7d0"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(outline);
    painter.setBrush(QColor("#166534"));
    painter.drawPolygon(QPolygonF{{30,39},{56,17},{82,39},{77,39},{77,67},{35,67},{35,39}});
    painter.drawRect(QRectF(42, 43, 11, 10));
    painter.drawRoundedRect(QRectF(60, 48, 11, 19), 1, 1);

    painter.setBrush(QColor("#15803d"));
    painter.drawPolygon(QPolygonF{{154,26},{174,19},{194,26},{192,46},{185,58},{174,65},{163,58},{156,46}});
    painter.drawPolyline(QPolygonF{{165,42},{171,48},{183,35}});

    painter.setBrush(QColor("#166534"));
    painter.drawPolygon(QPolygonF{{76,51},{87,38},{111,38},{123,51}});
    painter.drawRoundedRect(QRectF(69, 51, 64, 14), 5, 5);
    painter.drawEllipse(QRectF(78, 60, 11, 11));
    painter.drawEllipse(QRectF(114, 60, 11, 11));
    painter.drawLine(98, 39, 98, 50);

    painter.end();
    image.setDevicePixelRatio(2);
    return image;
}

enum AuthPage { SignInPage, RegistrationPage };

void markAuthInput(QLineEdit *input, bool invalid) {
    if (input->property("invalid").toBool() == invalid) return;
    input->setProperty("invalid", invalid);
    input->style()->unpolish(input);
    input->style()->polish(input);
}

void setAuthFeedback(QLabel *label, const QString &message, bool success = false) {
    label->setProperty("success", success);
    label->setText(message);
    label->style()->unpolish(label);
    label->style()->polish(label);
    label->setVisible(!message.isEmpty());
}

void documentHeader(QPainter &painter, int width, const QString &title, const QString &reference) {
    painter.fillRect(0, 0, width, 105, QColor("#14532d"));
    painter.setPen(Qt::white);
    painter.setFont(QFont("Segoe UI", 19, QFont::Bold));
    painter.drawText(30, 36, "Matseliso InsureRisk");
    painter.setFont(QFont("Segoe UI", 12));
    painter.drawText(30, 61, title);
    painter.setFont(QFont("Segoe UI", 9));
    painter.drawText(30, 84, painter.fontMetrics().elidedText("Policy reference " + reference,
                                                           Qt::ElideRight, width - 165));
    painter.drawText(QRect(30, 71, width - 60, 18), Qt::AlignRight,
                     QDate::currentDate().toString(Qt::ISODate));
    painter.setPen(QColor("#1e293b"));
    painter.setFont(QFont("Segoe UI", 11));
}
}

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
      QWidget { color:#1e293b; font-size:13px; }
      QMainWindow, QStackedWidget { background:#eef2f6; }
      QLabel { color:#1b2430; background:transparent; }
      QFrame#card, QFrame#kpi { background:#ffffff; border:1px solid #d9e1ea; border-radius:16px; }      QFrame#homeHero { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #166534,stop:1 #16a34a); border:0; border-radius:22px; }
      QFrame#homeHero QLabel { color:#ffffff; }
      QFrame#homeService { background:#ffffff; border:1px solid #d9e1ea; border-radius:20px; }
      QLabel#homeIcon { background:#dcfce7; border-radius:13px; }
      QLabel#homeCardTitle { font-size:17px; font-weight:700; }
      QPushButton#homeAction { background:#ffffff; color:#15803d; border:1px solid #22c55e; border-radius:8px; padding:9px 14px; font-weight:700; }
      QPushButton#homeAction:hover { background:#16a34a; color:#ffffff; border-color:#16a34a; }
      QPushButton#serviceCard { background:#ffffff; color:#1b2430; border:1px solid #d9e1ea; border-radius:18px; padding:22px; text-align:left; font-size:15px; font-weight:700; }
      QPushButton#serviceCard:hover, QPushButton#serviceCard:focus { background:#f0fdf4; border:2px solid #16a34a; color:#15803d; }
      QFrame#brandPanel { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #123b29,stop:1 #166534); border:0; border-radius:20px; }
      QFrame#brandPanel QLabel { color:#f0fdf4; }
      QLabel#authBrandLogo { background:#ffffff; border-radius:16px; color:#14532d; }
      QLabel#authBrandTitle, QLabel#authHeading { font-size:32px; font-weight:700; }
      QLabel#authBrandCopy { color:#d1fae5; font-size:15px; }
      QLabel#authEyebrow { color:#166534; font-size:11px; font-weight:700; }
      QStackedWidget#authForms { background:transparent; }
      QLabel#authFeedback { background:#fef2f2; color:#991b1b; padding:9px; border-radius:7px; }
      QLabel#authFeedback[success="true"] { background:#dcfce7; color:#14532d; }
      QLabel#title { font-size:17px; font-weight:700; }
      QLabel#caption { color:#64748b; font-size:11px; font-weight:700; }
      QLabel#value { font-size:24px; font-weight:700; }
      QLabel#muted { color:#64748b; }
      QLabel#modelRow { background:#f3f6fa; padding:11px; border-radius:7px; }
      QLabel#pill { background:#dcfce7; color:#15803d; border-radius:18px; padding:8px 18px; font-weight:700; }
      QLineEdit, QComboBox, QTextEdit { background:#ffffff; color:#1b2430; border:1px solid #d9e1ea; border-radius:7px; padding:8px; min-height:20px; }
      QLineEdit:focus, QComboBox:focus { border:2px solid #16a34a; padding:7px; }
      QPushButton { background:#ffffff; color:#1b2430; border:1px solid #d9e1ea; border-radius:7px; padding:10px 16px; font-weight:700; min-height:18px; }
      QPushButton#GreenPrimaryBtn { background:#166534; color:white; border-color:#166534; }
      QPushButton#GreenPrimaryBtn:hover { background:#15803d; }
      QPushButton#YellowSecondaryBtn { background:#eab308; color:#1b2430; border-color:#eab308; }
      QPushButton#RedDangerBtn { background:#dc2626; color:white; border-color:#dc2626; }
      QPushButton#BackBtn { background:#ffffff; color:#1b2430; border-color:#d9e1ea; }
      QPushButton#authLink { color:#166534; background:transparent; border-color:transparent; padding:5px 8px; }
      QPushButton#authLink:hover { text-decoration:underline; }
      QPushButton#authLink:focus { border:2px solid #0f766e; }
      QPushButton:focus, QCheckBox:focus, QSlider:focus, QTabBar::tab:focus { border:2px solid #0f766e; }
      QPushButton:disabled, QPushButton#GreenPrimaryBtn:disabled, QPushButton#RedDangerBtn:disabled {
          background:#e2e8f0; color:#475569; border-color:#cbd5e1;
      }
      QPushButton:checked { background:#dcfce7; color:#14532d; border-color:#15803d; }
      QFrame#quoteOption { background:#f8fafc; border:1px solid #cbd5e1; border-radius:12px; }
      QFrame#quoteOption[recommended="true"] { background:#ecfdf5; border:1px solid #15803d; }
      QLabel#quotePrice { font-size:22px; font-weight:700; }
      QLabel#previewPanel, QLabel#pill[decision="accept"] { background:#dcfce7; color:#14532d; border-radius:8px; padding:9px; }
      QLabel#pill[decision="refer"] { background:#fef3c7; color:#78350f; }
      QLabel#pill[decision="decline"] { background:#fee2e2; color:#991b1b; }
      QLineEdit[invalid="true"] { border:2px solid #b91c1c; }
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
        auto *caption = text(label, "caption");
        caption->setBuddy(input);
        input->setAccessibleName(label);
        l->addWidget(caption); l->addWidget(input); return box;
    };
    auto kpi = [&card, &text](const QString &caption, const QString &value, const QString &sub, const QString &colour = QString()) {
        auto *f = card("kpi"); auto *l = new QVBoxLayout(f); l->setContentsMargins(18,15,18,15); l->setSpacing(3);
        l->addWidget(text(caption.toUpper(), "caption")); auto *v=text(value,"value"); if (!colour.isEmpty()) v->setStyleSheet("color:"+colour+";"); l->addWidget(v); l->addWidget(text(sub,"muted")); return f;
    };

    centralStack = new QStackedWidget(this); setCentralWidget(centralStack);

    // Sign-in and registration share the same illustration and theme controls.
    loginScreen = new QWidget;
    auto *loginOuter = new QVBoxLayout(loginScreen);
    loginOuter->setContentsMargins(54, 24, 54, 38);
    loginOuter->setSpacing(16);
    auto *loginHeader = new QHBoxLayout;
    loginHeader->addWidget(text("Matseliso InsureRisk  /  Officer portal", "caption"));
    loginHeader->addStretch();
    auto *authThemeBtn = new QPushButton("Dark mode");
    authThemeBtn->setObjectName("BackBtn");
    loginHeader->addWidget(authThemeBtn);
    loginOuter->addLayout(loginHeader);

    auto *loginRow = new QHBoxLayout;
    loginRow->setSpacing(24);
    auto *loginCard = card();
    auto *loginLayout = new QVBoxLayout(loginCard);
    loginLayout->setContentsMargins(34, 24, 34, 24);
    loginLayout->addStretch();
    authForms = new QStackedWidget;
    authForms->setObjectName("authForms");
    authForms->setMinimumWidth(350);
    authForms->setMaximumWidth(440);
    loginLayout->addWidget(authForms, 0, Qt::AlignHCenter);
    loginLayout->addStretch();

    const auto makeAuthForm = [this, &text, &field](AuthForm &form, bool creating) {
        auto *page = new QWidget;
        auto *layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(10);
        layout->addWidget(text(creating ? "NEW ACCOUNT" : "OFFICER WORKSPACE", "authEyebrow"));
        layout->addWidget(text(creating ? "Create your account" : "Welcome back", "authHeading"));
        auto *intro = text(creating ? "Choose a username and password for this workspace."
                                    : "Sign in to your InsureRisk workspace.", "muted");
        intro->setWordWrap(true);
        layout->addWidget(intro);
        layout->addSpacing(8);

        form.username = new QLineEdit;
        form.username->setPlaceholderText(creating ? "Choose a username" : "Enter your username");
        form.password = new QLineEdit;
        form.password->setPlaceholderText(creating ? "Choose a password" : "Enter your password");
        form.password->setEchoMode(QLineEdit::Password);
        layout->addWidget(field("Username", form.username));
        layout->addWidget(field("Password", form.password));
        if (creating) {
            form.confirmation = new QLineEdit;
            form.confirmation->setPlaceholderText("Re-enter your password");
            form.confirmation->setEchoMode(QLineEdit::Password);
            layout->addWidget(field("Confirm password", form.confirmation));
        }

        form.showPassword = new QCheckBox(creating ? "Show passwords" : "Show password");
        layout->addWidget(form.showPassword);
        connect(form.showPassword, &QCheckBox::toggled, this,
                [password = form.password, confirmation = form.confirmation](bool visible) {
            const auto mode = visible ? QLineEdit::Normal : QLineEdit::Password;
            password->setEchoMode(mode);
            if (confirmation) confirmation->setEchoMode(mode);
        });

        form.feedback = text("", "authFeedback");
        form.feedback->setTextFormat(Qt::PlainText);
        form.feedback->setWordWrap(true);
        form.feedback->hide();
        layout->addWidget(form.feedback);
        for (auto *input : {form.username, form.password, form.confirmation}) {
            if (!input) continue;
            connect(input, &QLineEdit::textChanged, input, [input, feedback = form.feedback] {
                markAuthInput(input, false);
                if (!feedback->property("success").toBool()) setAuthFeedback(feedback, QString());
            });
        }

        auto *submit = new QPushButton(creating ? "Create account" : "Sign in");
        submit->setObjectName("GreenPrimaryBtn");
        submit->setMinimumHeight(44);
        layout->addWidget(submit);
        const auto submitForm = creating ? &MainWindow::handleRegister : &MainWindow::handleLogin;
        connect(submit, &QPushButton::clicked, this, submitForm);
        connect(form.username, &QLineEdit::returnPressed, form.password,
                [password = form.password] { password->setFocus(); });
        if (creating) {
            connect(form.password, &QLineEdit::returnPressed, form.confirmation,
                    [confirmation = form.confirmation] { confirmation->setFocus(); });
            connect(form.confirmation, &QLineEdit::returnPressed, this, submitForm);
        } else {
            connect(form.password, &QLineEdit::returnPressed, this, submitForm);
        }

        auto *switchRow = new QHBoxLayout;
        switchRow->addStretch();
        if (!creating) switchRow->addWidget(text("New to InsureRisk?", "muted"));
        auto *switchPage = new QPushButton(creating ? "← Back to sign in" : "Create account");
        switchPage->setObjectName("authLink");
        switchRow->addWidget(switchPage);
        switchRow->addStretch();
        layout->addLayout(switchRow);
        connect(switchPage, &QPushButton::clicked, this, [this, creating] {
            if (!creating && registrationForm.username->text().isEmpty())
                registrationForm.username->setText(signInForm.username->text());
            showAuthPage(creating ? SignInPage : RegistrationPage);
        });
        return page;
    };
    authForms->addWidget(makeAuthForm(signInForm, false));
    authForms->addWidget(makeAuthForm(registrationForm, true));

    auto *brand = card("brandPanel");
    auto *brandLayout = new QVBoxLayout(brand);
    brandLayout->setContentsMargins(24, 25, 24, 25);
    brandLayout->setSpacing(16);
    brandLayout->addStretch();
    QPixmap pix(":/src/images/logo.png");
    auto *logo = text("", "authBrandLogo");
    logo->setFixedSize(78, 78);
    logo->setAlignment(Qt::AlignCenter);
    if (!pix.isNull()) logo->setPixmap(pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else logo->setText("IR");
    brandLayout->addWidget(logo, 0, Qt::AlignHCenter);
    auto *brandTitle = text("InsureRisk", "authBrandTitle");
    brandLayout->addWidget(brandTitle, 0, Qt::AlignHCenter);
    auto *brandArt = new QLabel;
    brandArt->setPixmap(insuranceBanner(QSize(392, 140)));
    brandLayout->addWidget(brandArt, 0, Qt::AlignHCenter);
    auto *brandCopy = text("Assess risk, compare quotes, and manage policies.", "authBrandCopy");
    brandCopy->setWordWrap(true);
    brandCopy->setAlignment(Qt::AlignCenter);
    brandLayout->addWidget(brandCopy);
    brandLayout->addStretch();
    loginRow->addWidget(loginCard, 1);
    loginRow->addWidget(brand, 1);
    loginOuter->addLayout(loginRow, 1);
    centralStack->addWidget(loginScreen);
    signInForm.username->setFocus();

    // Application chrome
    appScreen = new QWidget; auto *app = new QVBoxLayout(appScreen); app->setContentsMargins(30,14,30,0); app->setSpacing(8);
    auto *header = new QHBoxLayout; auto *headLogo=new QLabel; if (!pix.isNull()) headLogo->setPixmap(pix.scaled(42,42,Qt::KeepAspectRatio,Qt::SmoothTransformation)); auto *headBrand=text("<span style='font-size:23px;font-weight:700;color:#16a34a;'>Matseliso_InsureRisk</span><br><span style='color:#64748b;'>Enterprise Actuarial Portal</span>");
    auto *homeBtn=new QPushButton("Home"); homeBtn->setObjectName("BackBtn"); auto *themeBtn=new QPushButton("☾  Dark mode"); auto *userChip=text("<b>●  admin</b><br><span style='color:#64748b;'>Administrator</span>"); auto *logoutBtn=new QPushButton("Log out"); logoutBtn->setObjectName("BackBtn");
    backBtn = new QPushButton("← Back");
    backBtn->setObjectName("BackBtn");
    backBtn->setAccessibleName("Back to previous screen");
    backBtn->setShortcut(QKeySequence("Alt+Left"));
    backBtn->setToolTip("Return to the previous screen (Alt+Left)");
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::goBack);
    auto *learnBtn = new QPushButton("Learning Centre");
    learnBtn->setObjectName("BackBtn");
    header->addWidget(headLogo); header->addWidget(headBrand); header->addStretch(); header->addWidget(backBtn); header->addWidget(homeBtn); header->addWidget(learnBtn); header->addWidget(themeBtn); header->addWidget(userChip); header->addWidget(logoutBtn); app->addLayout(header);
    tabWidget = new QTabWidget;
    workspaceStack = new QStackedWidget; app->addWidget(workspaceStack,1); centralStack->addWidget(appScreen);

    // Simulator
    auto *simTab = new QWidget; auto *simRow = new QHBoxLayout(simTab); simRow->setContentsMargins(0,10,0,8); simRow->setSpacing(16);
    auto *formCard = card(); auto *formBox = new QVBoxLayout(formCard); formBox->setContentsMargins(20,20,20,20); formBox->setSpacing(10); formBox->addWidget(text("Policy & Client Details","title"));
    policyIdInput = new QLineEdit; policyIdInput->setPlaceholderText("POL-101");
    clientNameInput = new QLineEdit; clientNameInput->setPlaceholderText("Client full name");
    phoneInput = new QLineEdit; phoneInput->setPlaceholderText("+266 5123 4567");
    addressInput = new QLineEdit; addressInput->setPlaceholderText("Physical address");
    coverTypeCombo = new QComboBox;
    coverTypeCombo->addItem(insuranceIcon(InsuranceIcon::Shield), "Life Insurance");
    coverTypeCombo->addItem(insuranceIcon(InsuranceIcon::Motor), "Car Insurance");
    coverTypeCombo->addItem(insuranceIcon(InsuranceIcon::Property), "Property Insurance");
    coverTypeCombo->addItem(insuranceIcon(InsuranceIcon::Policy), "Asset Protection");
    ageInput = new QLineEdit; ageInput->setPlaceholderText("24");
    incomeInput = new QLineEdit; incomeInput->setPlaceholderText("9,822.00");
    loansInput = new QLineEdit; loansInput->setPlaceholderText("142,000.00");
    geoZoneCombo = new QComboBox; geoZoneCombo->addItems({"Urban", "Suburban", "Rural"});
    assetInput = new QLineEdit; assetInput->setPlaceholderText("40,000.00");
    riskParamInput = new QLineEdit; riskParamInput->setPlaceholderText("2 - Secure");
    riskParamInput->setToolTip("Enter a whole number, such as 2. See Application basics in the Learning Centre for an example.");
    riskParamLabel = text("HAZARD / SECURITY RATING", "caption");
    formBox->addWidget(field("POLICY ID",policyIdInput)); formBox->addWidget(field("CLIENT FULL NAME",clientNameInput)); formBox->addWidget(field("PHONE NUMBER",phoneInput)); formBox->addWidget(field("PHYSICAL ADDRESS",addressInput)); formBox->addWidget(field("COVER TYPE",coverTypeCombo));
    auto *mini=new QGridLayout; mini->setHorizontalSpacing(12); mini->setVerticalSpacing(10); mini->addWidget(field("AGE",ageInput),0,0); mini->addWidget(field("GEOGRAPHIC ZONE",geoZoneCombo),0,1); mini->addWidget(field("MONTHLY INCOME (M)",incomeInput),1,0); mini->addWidget(field("ACTIVE LOANS (M)",loansInput),1,1); mini->addWidget(field("ASSET VALUE (M)",assetInput),2,0); mini->addWidget(field("HAZARD / SECURITY",riskParamInput),2,1); formBox->addLayout(mini);
    for (auto *input : {policyIdInput, ageInput, incomeInput, loansInput, assetInput, riskParamInput}) {
        connect(input, &QLineEdit::textChanged, input, [input] {
            if (!input->property("invalid").toBool()) return;
            input->setProperty("invalid", false);
            input->style()->unpolish(input);
            input->style()->polish(input);
        });
    }
    auto *calcBtn = new QPushButton("▶ Run simulation");
    calcBtn->setObjectName("GreenPrimaryBtn");
    auto *clearBtn = new QPushButton("Clear form");
    auto *work = new QWidget; auto *workBox=new QVBoxLayout(work); workBox->setContentsMargins(0,0,0,0); workBox->setSpacing(12); auto *titleRow=new QHBoxLayout; titleRow->addWidget(text("Actuarial Analysis & Underwriting Decision","title")); titleRow->addStretch(); decisionBadge=text("Awaiting simulation","pill"); titleRow->addWidget(decisionBadge); workBox->addLayout(titleRow);
    simulationContext = text("Enter policy details to start an assessment.", "muted");
    simulationContext->setTextFormat(Qt::PlainText);
    simulationContext->setWordWrap(true);
    workBox->addWidget(simulationContext);
    auto *metrics = new QGridLayout;
    riskScoreKpi = kpi("Composite risk score", "—", "Score out of 100");
    expectedLossKpi = kpi("Expected annual loss", "M —", "Model estimate");
    varKpi = kpi("Value-at-Risk (95%)", "M —", "95th percentile of loss");
    monthlyPremiumKpi = kpi("Monthly premium", "M —", "Annual premium divided by 12");
    metrics->addWidget(riskScoreKpi, 0, 0);
    metrics->addWidget(expectedLossKpi, 0, 1);
    metrics->addWidget(varKpi, 1, 0);
    metrics->addWidget(monthlyPremiumKpi, 1, 1);
    metrics->setColumnStretch(0, 1);
    metrics->setColumnStretch(1, 1);
    workBox->addLayout(metrics);
    auto *report=card(); auto *reportBox=new QHBoxLayout(report); reportBox->setContentsMargins(20,18,20,18); auto *drivers=new QVBoxLayout; drivers->addWidget(text("Risk drivers (weighted points)","title")); auto driver=[this,&text,drivers](const QString &name){auto *row=new QWidget;auto *rl=new QHBoxLayout(row);rl->setContentsMargins(0,2,0,2);auto *label=text(name);label->setMinimumWidth(125);auto *bar=new QProgressBar;bar->setRange(0,25);bar->setValue(0);bar->setTextVisible(false);riskDriverBars.append(bar);rl->addWidget(label);rl->addWidget(bar,1);drivers->addWidget(row);}; driver("Age profile");driver("Debt-to-income");driver("Geographic zone");driver("Hazard / health");driver("Asset exposure"); drivers->addStretch(); auto *pricing=new QVBoxLayout; pricing->addWidget(text("Pricing breakdown","title")); resultsOutput=new QTextEdit; resultsOutput->setReadOnly(true); resultsOutput->setPlaceholderText("Run a simulation to see the pricing breakdown and underwriting notes."); pricing->addWidget(resultsOutput,1); reportBox->addLayout(drivers,1); reportBox->addLayout(pricing,1); workBox->addWidget(report,1);
    auto *progressLine = new QHBoxLayout;
    progressLine->addWidget(text("Simulation progress", "caption"));
    progressLine->addStretch();
    simulationStatus = text("Ready to run", "caption");
    progressLine->addWidget(simulationStatus);
    workBox->addLayout(progressLine);
    simProgressBar = new QProgressBar;
    simProgressBar->setRange(0, 100);
    workBox->addWidget(simProgressBar);

    auto *actions = new QHBoxLayout;
    quoteBtn = new QPushButton("↓ Quote PDF"); quoteBtn->setEnabled(false);
    acceptBtn = new QPushButton("✓ Accept & Save"); acceptBtn->setObjectName("GreenPrimaryBtn"); acceptBtn->setEnabled(false);
    referBtn = new QPushButton("Refer"); referBtn->setObjectName("YellowSecondaryBtn"); referBtn->setEnabled(false);
    rejectBtn = new QPushButton("Reject"); rejectBtn->setObjectName("RedDangerBtn"); rejectBtn->setEnabled(false);
    actions->addWidget(quoteBtn);
    actions->addStretch();
    actions->addWidget(rejectBtn);
    actions->addWidget(acceptBtn);
    referBtn->hide();
    workBox->addLayout(actions);

    auto *formScroll = new QScrollArea;
    formScroll->setWidgetResizable(true);
    formScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    formScroll->setWidget(formCard);
    auto *formColumn = new QVBoxLayout;
    formColumn->addWidget(formScroll, 1);
    auto *formActions = new QHBoxLayout;
    formActions->addWidget(calcBtn, 1);
    formActions->addWidget(clearBtn);
    formColumn->addLayout(formActions);
    simRow->addLayout(formColumn, 4);
    simRow->addWidget(work, 8);
    tabWidget->addTab(simTab, "01  Risk assessment");


    // Pricing & quotation: deliberately compact; no live systems/telemetry panel.
    auto *pricingTab = new QWidget;
    auto *pricingLayout = new QHBoxLayout(pricingTab);
    pricingLayout->setContentsMargins(0, 10, 0, 8);
    pricingLayout->setSpacing(16);
    auto *matrixCard = card();
    auto *matrixLayout = new QVBoxLayout(matrixCard);
    matrixLayout->setContentsMargins(20, 18, 20, 18);
    auto *matrixHead = new QHBoxLayout;
    matrixHead->addWidget(text("Compare quotes", "title"));
    matrixHead->addStretch();
    auto *generateQuotes = new QPushButton("Generate quotes"); generateQuotes->setObjectName("GreenPrimaryBtn");
    matrixHead->addWidget(generateQuotes);
    matrixLayout->addLayout(matrixHead);
    matrixLayout->addWidget(text("Three deductible options based on the latest simulation.", "muted"));
    struct QuoteCard {
        QFrame *frame;
        QLabel *name;
        QLabel *price;
        QLabel *details;
        QLabel *badge;
        QPushButton *button;
    };
    std::array<QuoteCard, 3> quoteCards;
    auto *options = new QHBoxLayout;
    options->setSpacing(12);
    auto *previewButtons = new QButtonGroup(matrixCard);
    for (int i = 0; i < static_cast<int>(quoteCards.size()); ++i) {
        auto *frame = card("quoteOption");
        auto *layout = new QVBoxLayout(frame);
        layout->setContentsMargins(14, 16, 14, 16);
        layout->setSpacing(9);
        auto *badge = text(QString("OPTION %1").arg(i + 1), "caption");
        auto *name = text("Quote pending", "title");
        auto *price = text("—", "quotePrice");
        auto *details = text("Annual premium and deductible will appear here.", "muted");
        details->setWordWrap(true);
        auto *button = new QPushButton("Preview quote");
        button->setCheckable(true);
        button->setEnabled(false);
        previewButtons->addButton(button, i);
        layout->addWidget(badge);
        layout->addWidget(name);
        layout->addWidget(price);
        layout->addWidget(details);
        layout->addStretch();
        layout->addWidget(button);
        options->addWidget(frame, 1);
        quoteCards[i] = {frame, name, price, details, badge, button};
    }
    matrixLayout->addSpacing(12);
    matrixLayout->addLayout(options);
    auto *quoteNote = text("Run the simulation, then select Generate quotes.", "muted");
    quoteNote->setWordWrap(true);
    matrixLayout->addWidget(quoteNote);
    matrixLayout->addStretch();
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
    auto *preview = text("QUOTE PREVIEW\nRun a simulation to calculate a premium.", "previewPanel");
    preview->setWordWrap(true);
    advisoryLayout->addWidget(preview);
    advisoryLayout->addWidget(text("Risk mitigation", "caption"));
    mitigationMeasures = RiskAdvisoryEngine::suggestions(latestScore);
    QVector<QCheckBox *> measureChecks;
    for (const auto &measure : mitigationMeasures) {
        auto *check = new QCheckBox(measure.name);
        check->setToolTip(measure.description);
        check->setEnabled(false);
        advisoryLayout->addWidget(check);
        measureChecks.push_back(check);
    }
    auto *impact = text("Generate quotes to calculate the impact of mitigation measures.", "muted");
    impact->setWordWrap(true);
    advisoryLayout->addWidget(impact);
    advisoryLayout->addStretch();
    pricingLayout->addWidget(advisoryCard, 1);
    tabWidget->addTab(pricingTab, "02  Pricing & quotation");

    const auto updateImpact = [this, measureChecks, impact]() {
        QVector<MitigationMeasure> selected;
        for (qsizetype i = 0; i < measureChecks.size() && i < mitigationMeasures.size(); ++i) {
            if (measureChecks[i]->isChecked()) selected.push_back(mitigationMeasures[i]);
        }
        const auto outcome = RiskAdvisoryEngine::combine(selected);
        impact->setText(selected.isEmpty() ? "Select measures to see their combined impact." :
            QString("Combined impact: -%1 risk points; save %2 / year")
                .arg(outcome.combinedRiskReduction, 0, 'f', 1)
                .arg(DynamicPricingEngine::money(outcome.combinedAnnualSaving)));
    };
    for (auto *check : measureChecks) {
        connect(check, &QCheckBox::toggled, this, updateImpact);
    }

    const auto showQuote = [this, preview](int index) {
        if (index < 0 || index >= latestPricing.tiers.size()) return;
        const auto &tier = latestPricing.tiers[index];
        preview->setText(QString("QUOTE PREVIEW\n%1\n%2 per month\n%3 per year\n%4% deductible")
            .arg(tier.name, DynamicPricingEngine::money(tier.monthlyPremium),
                 DynamicPricingEngine::money(tier.annualPremium), QString::number(tier.deductiblePercent, 'g')));
    };
    connect(previewButtons, &QButtonGroup::idClicked, this, showQuote);

    auto updatePricing = [this, quoteCards, quoteNote, showQuote, deductibleSlider, marginSlider, loadingSlider,
                          deductibleValue, marginValue, loadingValue, measureChecks, updateImpact]() {
        deductibleValue->setText(QString::number(deductibleSlider->value()) + "%");
        marginValue->setText(QString::number(marginSlider->value()) + "%");
        loadingValue->setText(QString::number(loadingSlider->value()) + "%");
        if (latestExpectedLoss <= 0.0) { quoteNote->setText("Run the simulation first to prepare a quotation."); return; }
        PricingRequest request{latestExpectedLoss, latestExpectedLoss + latestCapitalCharge / .06, latestScore,
                               static_cast<double>(marginSlider->value()), static_cast<double>(loadingSlider->value()),
                               static_cast<double>(deductibleSlider->value())};
        latestPricing = DynamicPricingEngine::calculate(request);
        for (int row = 0; row < static_cast<int>(quoteCards.size()) && row < latestPricing.tiers.size(); ++row) {
            const auto &tier = latestPricing.tiers[row];
            const auto &option = quoteCards[row];
            option.name->setText(tier.name);
            option.price->setText(DynamicPricingEngine::money(tier.monthlyPremium));
            option.details->setText(QString("per month\n%1 per year\n%2% deductible")
                .arg(DynamicPricingEngine::money(tier.annualPremium), QString::number(tier.deductiblePercent, 'g')));
            option.badge->setText(tier.recommended ? "RECOMMENDED" : QString("OPTION %1").arg(row + 1));
            option.frame->setProperty("recommended", tier.recommended);
            option.frame->style()->unpolish(option.frame);
            option.frame->style()->polish(option.frame);
            option.button->setEnabled(true);
            option.button->setAccessibleName("Preview " + tier.name);
            if (tier.recommended) {
                option.button->setChecked(true);
                showQuote(row);
            }
        }
        mitigationMeasures = RiskAdvisoryEngine::suggestions(latestScore);
        for (auto *check : measureChecks) check->setEnabled(true);
        updateImpact();
        quoteNote->setText(latestScore >= 55.0
            ? "The pricing model recommends the high deductible for a risk score of 55 or higher."
            : "The pricing model recommends the medium deductible for a risk score below 55.");
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
    recordsRow->addWidget(drawer, 1);
    recordsBox->addLayout(recordsRow, 1);
    recordsBox->addWidget(text("Saved policy records", "muted"));
    tabWidget->addTab(recordsTab, "03  Policy records");
// Analytics
    auto *analyticsTab = new QWidget;
    auto *analyticsBox = new QVBoxLayout(analyticsTab);
    analyticsBox->setContentsMargins(0, 10, 0, 8);
    auto *analyticsHead = new QHBoxLayout;
    analyticsHead->addWidget(text("Insurance portfolio", "title"));
    analyticsHead->addStretch();
    analyticsHead->addWidget(text("All saved records", "caption"));
    auto *refreshAnalyticsBtn = new QPushButton("Refresh analytics");
    refreshAnalyticsBtn->setObjectName("GreenPrimaryBtn");
    analyticsHead->addWidget(refreshAnalyticsBtn);
    analyticsBox->addLayout(analyticsHead);

    auto *analyticsKpis = new QHBoxLayout;
    const std::array<QFrame *, 5> portfolioCards = {
        kpi("Applications", "—", "Saved records"),
        kpi("Approval rate", "—", "Decided records"),
        kpi("Average risk", "—", "Approved records"),
        kpi("Annual premiums", "—", "Approved records"),
        kpi("Average VaR 95%", "—", "Approved records")
    };
    for (auto *metric : portfolioCards) analyticsKpis->addWidget(metric, 1);
    analyticsBox->addLayout(analyticsKpis);

    auto *charts = new QGridLayout;
    auto *outcomes = card();
    auto *outcomesBox = new QVBoxLayout(outcomes);
    outcomesBox->addWidget(text("Underwriting outcomes", "title"));
    chartView = new QChartView(new QChart);
    chartView->chart()->setBackgroundVisible(false);
    chartView->chart()->legend()->hide();
    chartView->chart()->setMargins(QMargins());
    chartView->setMinimumHeight(125);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setAccessibleName("Approved and rejected applications");
    outcomesBox->addWidget(chartView, 1);
    analyticsSummaryLabel = text("Refresh to load underwriting outcomes.", "muted");
    analyticsSummaryLabel->setWordWrap(true);
    outcomesBox->addWidget(analyticsSummaryLabel);
    charts->addWidget(outcomes, 0, 0);

    using BarRows = std::array<QPair<QProgressBar *, QLabel *>, 4>;
    const QStringList coverNames = {"Life", "Motor", "Property", "Asset protection"};
    const QStringList tierNames = {"Low · below 30", "Moderate · 30 to <55", "High · 55 to <75", "Very high · 75+"};
    const auto barCard = [&card, &text, charts](const QString &title, const QStringList &labels, int row, int column) {
        auto *panel = card();
        auto *layout = new QVBoxLayout(panel);
        layout->addWidget(text(title, "title"));
        BarRows bars{};
        for (int i = 0; i < labels.size(); ++i) {
            auto *heading = new QHBoxLayout;
            heading->addWidget(text(labels[i], "caption"));
            auto *value = text("—");
            heading->addWidget(value, 0, Qt::AlignRight);
            layout->addLayout(heading);
            auto *bar = new QProgressBar;
            bar->setTextVisible(false);
            bar->setValue(0);
            bar->setAccessibleName(labels[i] + " " + title);
            layout->addWidget(bar);
            bars[i] = {bar, value};
        }
        layout->addStretch();
        charts->addWidget(panel, row, column);
        return bars;
    };
    const auto premiumBars = barCard("Annual premium by cover", coverNames, 0, 1);
    const auto tierBars = barCard("Risk tier distribution", tierNames, 0, 2);
    const auto varBars = barCard("Average policy VaR 95%", coverNames, 1, 2);

    auto *recent = card();
    auto *recentBox = new QVBoxLayout(recent);
    recentBox->addWidget(text("Recent applications", "title"));
    auto *recentList = text("No applications loaded.", "muted");
    recentList->setTextFormat(Qt::PlainText);
    recentList->setWordWrap(true);
    recentBox->addWidget(recentList);
    recentBox->addStretch();
    charts->addWidget(recent, 1, 0, 1, 2);
    for (int i = 0; i < 3; ++i) charts->setColumnStretch(i, 1);
    analyticsBox->addLayout(charts, 1);
    tabWidget->addTab(analyticsTab, "04  Portfolio");


    // Home: a focused launcher, not a dashboard full of widgets.
    homePage = new QWidget;
    auto *homeLayout = new QVBoxLayout(homePage);
    homeLayout->setContentsMargins(64, 24, 64, 24);
    homeLayout->setSpacing(14);
    auto *hero = new QFrame; hero->setObjectName("homeHero");
    auto *heroLayout = new QHBoxLayout(hero);
    heroLayout->setContentsMargins(30, 18, 24, 18);
    heroLayout->setSpacing(20);
    auto *heroCopy = new QVBoxLayout;
    auto *heroTitle = new QLabel("Insurance decisions made clear"); heroTitle->setStyleSheet("font-size:28px;font-weight:700;color:#ffffff;");
    auto *heroText = new QLabel("Assess an application, compare quotes, or review your insurance portfolio."); heroText->setStyleSheet("font-size:14px;color:#dcfce7;");
    heroText->setWordWrap(true);
    heroTitle->setWordWrap(true);
    heroCopy->addWidget(heroTitle);
    heroCopy->addWidget(heroText);
    heroLayout->addLayout(heroCopy, 1);
    auto *heroArt = new QLabel;
    heroArt->setPixmap(insuranceBanner());
    heroLayout->addWidget(heroArt, 0, Qt::AlignVCenter);
    homeLayout->addWidget(hero);

    auto *learningCard = card();
    auto *learningRow = new QHBoxLayout(learningCard);
    learningRow->setContentsMargins(18, 12, 18, 12);
    auto *learningCopy = new QVBoxLayout;
    learningCopy->addWidget(text("Learning Centre", "title"));
    auto *learningDescription = text("Explore visual lessons and try insurance decisions in practice mode.", "muted");
    learningDescription->setWordWrap(true);
    learningCopy->addWidget(learningDescription);
    learningRow->addLayout(learningCopy, 1);
    auto *openLearning = new QPushButton("Start learning");
    openLearning->setObjectName("GreenPrimaryBtn");
    learningRow->addWidget(openLearning);
    homeLayout->addWidget(learningCard);
    homeLayout->addWidget(text("What would you like to do?", "title"));

    auto *serviceGrid = new QGridLayout;
    serviceGrid->setHorizontalSpacing(20); serviceGrid->setVerticalSpacing(20);
    auto makeService = [this, &text](InsuranceIcon symbol, const QString &title, const QString &description, const QString &actionText) {
        auto *frame = new QFrame; frame->setObjectName("homeService"); frame->setMinimumSize(330, 160);
        auto *layout = new QVBoxLayout(frame); layout->setContentsMargins(20, 13, 20, 13); layout->setSpacing(6);
        auto *top = new QHBoxLayout; auto *icon = text("", "homeIcon"); icon->setAlignment(Qt::AlignCenter); icon->setFixedSize(46,46);
        icon->setPixmap(insuranceIcon(symbol).pixmap(QSize(24, 24), devicePixelRatioF()));
        top->addWidget(icon); top->addStretch(); layout->addLayout(top);
        auto *heading = text(title); heading->setObjectName("homeCardTitle"); layout->addWidget(heading);
        auto *body = text(description, "muted"); body->setWordWrap(true); layout->addWidget(body); layout->addStretch();
        auto *action = new QPushButton(actionText); action->setObjectName("homeAction"); layout->addWidget(action, 0, Qt::AlignLeft);
        return qMakePair(frame, action);
    };
    const auto dashboardService = makeService(InsuranceIcon::Shield, "Risk assessment",
        "Review an application and run an actuarial simulation.", "Assess an application");
    const auto pricingService = makeService(InsuranceIcon::Quote, "Pricing and quotation",
        "Compare three premium and deductible options.", "Compare quotes");
    const auto recordsService = makeService(InsuranceIcon::Policy, "Policy records",
        "Review saved policies and create billing statements.", "View policies");
    const auto portfolioService = makeService(InsuranceIcon::Portfolio, "Portfolio overview",
        "Review underwriting outcomes, premiums, and risk across saved records.", "View portfolio");
    serviceGrid->addWidget(dashboardService.first, 0, 0);
    serviceGrid->addWidget(pricingService.first, 0, 1);
    serviceGrid->addWidget(recordsService.first, 1, 0);
    serviceGrid->addWidget(portfolioService.first, 1, 1);
    homeLayout->addLayout(serviceGrid);
    homeLayout->addStretch();

    workspaceStack->addWidget(homePage);
    workspaceStack->addWidget(tabWidget);
    learningCentre = new LearningCentre;
    workspaceStack->addWidget(learningCentre);
    showWorkspace(homePage);
    const auto showLearning = [this] { navigateTo(learningCentre); };
    connect(learnBtn, &QPushButton::clicked, this, showLearning);
    connect(openLearning, &QPushButton::clicked, this, showLearning);
    connect(homeBtn, &QPushButton::clicked, this, [this] { navigateTo(homePage); });
    connect(dashboardService.second, &QPushButton::clicked, this, [this, simTab] { navigateTo(simTab); });
    connect(pricingService.second, &QPushButton::clicked, this, [this, pricingTab] { navigateTo(pricingTab); });
    connect(recordsService.second, &QPushButton::clicked, this, [this, recordsTab] { navigateTo(recordsTab); });
    connect(portfolioService.second, &QPushButton::clicked, this, [this, analyticsTab] { navigateTo(analyticsTab); });
    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index >= 0 && workspaceStack->currentWidget() == tabWidget) {
            navigateTo(tabWidget->widget(index));
        }
    });
    connect(logoutBtn,&QPushButton::clicked,this,&MainWindow::handleLogout); connect(calcBtn,&QPushButton::clicked,this,&MainWindow::runSimulation); connect(acceptBtn,&QPushButton::clicked,this,&MainWindow::acceptApplication); connect(rejectBtn,&QPushButton::clicked,this,&MainWindow::rejectApplication); connect(refreshBtn,&QPushButton::clicked,this,&MainWindow::refreshDatabaseTable);
    connect(clearBtn, &QPushButton::clicked, this, [this] {
        for (auto *input : {policyIdInput, clientNameInput, phoneInput, addressInput, ageInput,
                            incomeInput, loansInput, assetInput, riskParamInput}) input->clear();
        resultsOutput->clear();
        simProgressBar->setValue(0);
        simulationStatus->setText("Ready to run");
        simulationContext->setText("Enter policy details to start an assessment.");
    });

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
        QPdfWriter writer(file);
        writer.setResolution(72);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageMargins(QMarginsF(15,15,15,15));
        QPainter painter(&writer);
        const int w = writer.width(), m = 30;
        documentHeader(painter, w, "Billing statement", cell(0));
        int y = 155;
        const QList<QPair<QString,QString>> rows={{"Policy ID",cell(0)},{"Client",cell(1)},{"Cover",cell(3)},{"Policy status",cell(7)},{"Monthly premium",DynamicPricingEngine::money(cell(5).toDouble())},{"Annual premium",DynamicPricingEngine::money(cell(6).toDouble())}};
        for (const auto &entry : rows) {
            painter.setPen(QColor("#d9e1ea"));
            painter.drawLine(m, y - 22, w - m, y - 22);
            painter.setPen(QColor("#1e293b"));
            painter.drawText(QRect(m, y - 17, 155, 22), Qt::AlignVCenter, entry.first);
            painter.drawText(QRect(m + 155, y - 17, w - 2*m - 155, 22), Qt::AlignRight | Qt::AlignVCenter, entry.second);
            y += 52;
        }
        painter.end();
        QMessageBox::information(this, "Statement created", "The billing statement was saved.");
    });
    connect(recordsTable,&QTableWidget::cellChanged,this,[this](int row,int){ auto *state=recordsTable->item(row,7); if(state && state->text()=="Rejected"){ QMessageBox::warning(this,"Restriction","Rejected applications are read-only."); refreshDatabaseTable(); }});
    const auto refreshPortfolio = [this, portfolioCards, premiumBars, tierBars, varBars, recentList]() {
        sqlite3 *db = nullptr;
        if (sqlite3_open_v2("insurerisk.db", &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
            if (db) sqlite3_close(db);
            analyticsSummaryLabel->setText("Unable to read the portfolio database.");
            return;
        }
        sqlite3_stmt *stmt = nullptr;
        const char *query = "SELECT policy_id, cover_type, status, annual_premium, risk_score, var_95 "
                            "FROM policies ORDER BY id DESC;";
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            analyticsSummaryLabel->setText("Unable to load portfolio metrics.");
            return;
        }

        const QStringList covers = {"Life Insurance", "Car Insurance", "Property Insurance", "Asset Protection"};
        std::array<double, 4> premiums{}, tiers{}, varSums{};
        std::array<int, 4> coverCounts{};
        int applications = 0, approved = 0, rejected = 0;
        double totalPremium = 0.0, totalRisk = 0.0, totalVar = 0.0;
        QStringList recentRows;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const auto stringAt = [stmt](int column) {
                return QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(stmt, column)));
            };
            const QString cover = stringAt(1);
            const QString status = stringAt(2);
            const double score = sqlite3_column_double(stmt, 4);
            ++applications;
            ++tiers[score < 30 ? 0 : score < 55 ? 1 : score < 75 ? 2 : 3];
            if (recentRows.size() < 5) {
                recentRows << QString("%1  ·  %2  ·  %3")
                    .arg(stringAt(0), cover, status == "Accepted" ? QString("Approved") : status);
            }
            if (status == "Rejected") ++rejected;
            if (status != "Approved" && status != "Accepted") continue;

            ++approved;
            const double premium = sqlite3_column_double(stmt, 3);
            const double var = sqlite3_column_double(stmt, 5);
            totalPremium += premium;
            totalRisk += score;
            totalVar += var;
            const qsizetype index = covers.indexOf(cover);
            if (index >= 0) {
                premiums[index] += premium;
                varSums[index] += var;
                ++coverCounts[index];
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);

        const int decided = approved + rejected;
        const std::array<QString, 5> values = {
            QString::number(applications),
            decided ? QString::number(100.0 * approved / decided, 'f', 1) + "%" : QString("—"),
            approved ? QString::number(totalRisk / approved, 'f', 1) : QString("—"),
            DynamicPricingEngine::money(totalPremium),
            approved ? DynamicPricingEngine::money(totalVar / approved) : QString("—")
        };
        for (size_t i = 0; i < values.size(); ++i)
            portfolioCards[i]->findChild<QLabel *>("value")->setText(values[i]);

        for (size_t i = 0; i < varSums.size(); ++i)
            if (coverCounts[i]) varSums[i] /= coverCounts[i];

        const auto updateBars = [](const BarRows &bars, const std::array<double, 4> &values, bool currency) {
            const double scale = *std::max_element(values.cbegin(), values.cend());
            for (size_t i = 0; i < bars.size(); ++i) {
                const QString label = currency ? DynamicPricingEngine::money(values[i]) : QString::number(values[i]);
                bars[i].first->setValue(scale > 0.0 ? qRound(100 * values[i] / scale) : 0);
                bars[i].first->setAccessibleDescription(label);
                bars[i].second->setText(label);
            }
        };
        updateBars(premiumBars, premiums, true);
        updateBars(tierBars, tiers, false);
        updateBars(varBars, varSums, true);
        recentList->setText(recentRows.isEmpty() ? QString("No applications saved yet.") : recentRows.join('\n'));

        auto *chart = chartView->chart();
        chart->removeAllSeries();
        auto *outcomeSeries = new QPieSeries;
        outcomeSeries->setHoleSize(.60);
        outcomeSeries->setPieSize(.86);
        if (approved) outcomeSeries->append("Approved", approved)->setColor(QColor("#16803f"));
        if (rejected) outcomeSeries->append("Rejected", rejected)->setColor(QColor("#be3141"));
        if (decided) chart->addSeries(outcomeSeries);
        else delete outcomeSeries;
        analyticsSummaryLabel->setText(decided
            ? QString("%1 approved  ·  %2 rejected").arg(approved).arg(rejected)
            : QString("No underwriting decisions to chart yet."));
    };
    connect(refreshAnalyticsBtn, &QPushButton::clicked, this, refreshPortfolio);
    connect(tabWidget, &QTabWidget::currentChanged, this, [tabWidget = tabWidget, analyticsTab, refreshPortfolio](int index) {
        if (tabWidget->widget(index) == analyticsTab) refreshPortfolio();
    });

    const QString lightStyle = styleSheet();
    const auto applyTheme = [this, themeBtn, authThemeBtn, lightStyle](bool dark) {
        themeBtn->setText(dark ? "Light mode" : "Dark mode");
        authThemeBtn->setText(themeBtn->text());
        setStyleSheet(dark ? lightStyle + R"(
          QWidget { color:#e2e8f0; }
          QMainWindow, QStackedWidget, QTabWidget::pane { background:#0f172a; }
          QFrame#card, QFrame#kpi, QFrame#homeService, QFrame#quoteOption { background:#172338; border-color:#334155; }
          QFrame#quoteOption[recommended="true"] { background:#102d25; border-color:#4ade80; }
          QLabel { color:#e2e8f0; }
          QLabel#caption, QLabel#muted, QTabBar::tab { color:#b8c6d8; }
          QTabBar::tab:selected { color:#86efac; border-color:#4ade80; }
          QLineEdit, QComboBox, QTextEdit, QTableWidget, QComboBox QAbstractItemView {
              background:#101a2d; color:#f1f5f9; border-color:#475569;
              alternate-background-color:#192740;
          }
          QTableWidget::item { color:#f1f5f9; }
          QTableWidget::item:selected { background:#14532d; color:#f0fdf4; }
          QHeaderView::section, QPushButton, QPushButton#BackBtn {
              background:#1e293b; color:#f1f5f9; border-color:#475569;
          }
          QPushButton:disabled, QPushButton#GreenPrimaryBtn:disabled, QPushButton#RedDangerBtn:disabled {
              background:#273449; color:#b8c6d8; border-color:#475569;
          }
          QPushButton:checked { background:#14532d; color:#dcfce7; border-color:#4ade80; }
          QLineEdit[invalid="true"] { border:2px solid #e26666; }
          QLabel#previewPanel, QLabel#pill[decision="accept"] { background:#14532d; color:#dcfce7; }
          QLabel#pill[decision="refer"] { background:#42260c; color:#fde68a; }
          QLabel#pill[decision="decline"] { background:#481a26; color:#fecdd3; }
          QLabel#modelRow, QProgressBar { background:#334155; }
          QLabel#authEyebrow, QPushButton#authLink { color:#86efac; }
          QLabel#authFeedback { background:#481a26; color:#fecdd3; }
          QLabel#authFeedback[success="true"] { background:#14532d; color:#dcfce7; }
        )" : lightStyle);
        chartView->setBackgroundBrush(dark ? QColor("#1c2b45") : QColor("#ffffff"));
        learningCentre->setDarkMode(dark);
    };
    darkTheme = DatabaseManager::getSetting("theme", "light") == "dark";
    applyTheme(darkTheme);
    const auto toggleTheme = [this, applyTheme] {
        darkTheme = !darkTheme;
        DatabaseManager::setSetting("theme", darkTheme ? "dark" : "light");
        applyTheme(darkTheme);
    };
    connect(themeBtn, &QPushButton::clicked, this, toggleTheme);
    connect(authThemeBtn, &QPushButton::clicked, this, toggleTheme);
    connect(csv,&QPushButton::clicked,this,[this,analyticsTab](){const QString file=QFileDialog::getSaveFileName(this,"Export policy records",QString(),"CSV files (*.csv)");if(file.isEmpty())return;QSaveFile out(file);if(!out.open(QIODevice::WriteOnly|QIODevice::Text)){QMessageBox::warning(this,"Export failed","Unable to write the selected file.");return;}QTextStream stream(&out);for(int c=0;c<recordsTable->columnCount();++c){if(c)stream<<",";stream<<recordsTable->horizontalHeaderItem(c)->text();}stream<<"\n";for(int r=0;r<recordsTable->rowCount();++r){for(int c=0;c<recordsTable->columnCount();++c){if(c)stream<<",";auto *item=recordsTable->item(r,c);stream<<(item?item->text():QString());}stream<<"\n";}if(out.commit())QMessageBox::information(this,"CSV export","Policy records exported successfully.");});
    connect(quoteBtn,&QPushButton::clicked,this,[this,analyticsTab](){
        if(latestReport.isEmpty()) return;
        const QString file=QFileDialog::getSaveFileName(this,"Save actuarial quote",QString("actuarial-quote.pdf"),"PDF files (*.pdf)");
        if(file.isEmpty()) return;
        QPdfWriter writer(file); writer.setResolution(72); writer.setPageSize(QPageSize(QPageSize::A4)); writer.setPageMargins(QMarginsF(15,15,15,15));
        QPainter painter(&writer); const int w = writer.width(), margin = 30;

        documentHeader(painter, w, "Actuarial quote", policyIdInput->text());
        int y = 145;
        painter.setPen(QColor("#1e293b"));
        painter.drawText(QRect(margin, y - 17, 100, 22), Qt::AlignVCenter, "Policy ID");
        painter.drawText(QRect(margin + 100, y - 17, w - 2*margin - 100, 22),
                         Qt::AlignRight | Qt::AlignVCenter, policyIdInput->text());
        y += 42;
        painter.drawText(QRect(margin, y - 17, 100, 22), Qt::AlignVCenter, "Client");
        painter.drawText(QRect(margin + 100, y - 17, w - 2*margin - 100, 22),
                         Qt::AlignRight | Qt::AlignVCenter, clientNameInput->text());
        y += 55;
        const QList<QPair<QString,QString>> rows={{"Expected annual loss",DynamicPricingEngine::money(latestExpectedLoss)},{"Risk-loaded loss",DynamicPricingEngine::money(latestRiskLoadedLoss)},{"Capital charge",DynamicPricingEngine::money(latestCapitalCharge)},{"Administration fee",DynamicPricingEngine::money(150.0)}};
        painter.fillRect(margin,y,w-margin*2,34,QColor("#f3f6fa")); painter.setFont(QFont("Segoe UI",11,QFont::Bold)); painter.drawText(margin+12,y+23,"Pricing item"); painter.drawText(QRect(margin+280,y,w-margin-(margin+280),34),Qt::AlignRight|Qt::AlignVCenter,"Amount"); y+=34;
        painter.setFont(QFont("Segoe UI",11));
        for(const auto &row:rows){ painter.setPen(QColor("#d9e1ea")); painter.drawLine(margin,y,w-margin,y); painter.setPen(QColor("#1b2430")); painter.drawText(margin+12,y+28,row.first); painter.drawText(QRect(margin+280,y,w-margin-(margin+280),42),Qt::AlignRight|Qt::AlignVCenter,row.second); y+=42; }
        painter.setPen(QColor("#16a34a")); painter.fillRect(margin,y,w-margin*2,52,QColor("#ecfdf3")); painter.setFont(QFont("Segoe UI",12,QFont::Bold)); painter.drawText(margin+12,y+32,"Annual premium"); painter.drawText(QRect(margin+200,y,w-2*margin-212,52),Qt::AlignRight | Qt::AlignVCenter,DynamicPricingEngine::money(latestAnnualPremium)); y+=72;
        painter.fillRect(margin,y,w-margin*2,52,QColor("#ecfdf3")); painter.setPen(QColor("#15803d")); painter.drawText(margin+12,y+32,"Monthly premium"); painter.drawText(QRect(margin+200,y,w-2*margin-212,52),Qt::AlignRight | Qt::AlignVCenter,DynamicPricingEngine::money(latestMonthlyPremium)); y+=86;
        painter.setPen(QColor("#1b2430")); painter.setFont(QFont("Segoe UI",11,QFont::Bold)); painter.drawText(margin,y,"Underwriting summary"); y+=27; painter.setFont(QFont("Segoe UI",10)); painter.drawText(QRect(margin,y,w-margin*2,80),Qt::TextWordWrap,"This quote is based on the policy details supplied. The assessed risk score is "+QString::number(latestScore,'f',2)+". Please contact your underwriter if you need help understanding your cover or premium.");
        painter.end(); QMessageBox::information(this,"Quote created","Your structured actuarial quote was saved.");
    });

}
void MainWindow::navigateTo(QWidget *page) {
    if (page == currentWorkspace) return;
    workspaceHistory.push_back(currentWorkspace);
    showWorkspace(page);
}

void MainWindow::showWorkspace(QWidget *page) {
    currentWorkspace = page;
    if (tabWidget->indexOf(page) >= 0) {
        tabWidget->setCurrentWidget(page);
        workspaceStack->setCurrentWidget(tabWidget);
    } else {
        workspaceStack->setCurrentWidget(page);
    }
    backBtn->setEnabled(!workspaceHistory.isEmpty());
}

void MainWindow::goBack() {
    if (!workspaceHistory.isEmpty()) showWorkspace(workspaceHistory.takeLast());
}

void MainWindow::clearAuthFeedback(AuthForm &form) {
    for (auto *input : {form.username, form.password, form.confirmation}) {
        if (input) markAuthInput(input, false);
    }
    setAuthFeedback(form.feedback, QString());
}

void MainWindow::showAuthPage(int index) {
    for (auto *form : {&signInForm, &registrationForm}) {
        form->showPassword->setChecked(false);
        form->password->clear();
        if (form->confirmation) form->confirmation->clear();
        clearAuthFeedback(*form);
    }
    authForms->setCurrentIndex(index);
    auto &active = index == RegistrationPage ? registrationForm : signInForm;
    active.username->setFocus();
}

void MainWindow::handleLogin() {
    auto &form = signInForm;
    clearAuthFeedback(form);
    const QString username = form.username->text();
    const QString password = form.password->text();
    if (username.isEmpty() || password.isEmpty()) {
        markAuthInput(form.username, username.isEmpty());
        markAuthInput(form.password, password.isEmpty());
        setAuthFeedback(form.feedback, "Enter your username and password.");
        (username.isEmpty() ? form.username : form.password)->setFocus();
        return;
    }
    if (!AuthManager::loginUser(username.toStdString(), password.toStdString())) {
        markAuthInput(form.username, true);
        markAuthInput(form.password, true);
        setAuthFeedback(form.feedback, "The username or password is incorrect. Check your details and try again.");
        form.password->setFocus();
        form.password->selectAll();
        return;
    }
    currentUser = username;
    learningCentre->setLearner(currentUser);
    form.showPassword->setChecked(false);
    form.password->clear();
    centralStack->setCurrentIndex(1);
    refreshDatabaseTable();
}

void MainWindow::handleRegister() {
    auto &form = registrationForm;
    clearAuthFeedback(form);
    const QString username = form.username->text();
    const QString password = form.password->text();
    const QString confirmation = form.confirmation->text();
    if (username.isEmpty() || password.isEmpty() || confirmation.isEmpty()) {
        markAuthInput(form.username, username.isEmpty());
        markAuthInput(form.password, password.isEmpty());
        markAuthInput(form.confirmation, confirmation.isEmpty());
        setAuthFeedback(form.feedback, "Enter a username, password, and password confirmation.");
        auto *firstEmpty = username.isEmpty() ? form.username : password.isEmpty() ? form.password : form.confirmation;
        firstEmpty->setFocus();
        return;
    }
    if (password != confirmation) {
        markAuthInput(form.confirmation, true);
        setAuthFeedback(form.feedback, "The passwords don’t match. Re-enter your password to confirm it.");
        form.confirmation->setFocus();
        form.confirmation->selectAll();
        return;
    }
    if (!AuthManager::registerUser(username.toStdString(), password.toStdString())) {
        setAuthFeedback(form.feedback, "Could not create the account. Check whether the username is already in use.");
        form.username->setFocus();
        return;
    }
    signInForm.username->setText(username);
    registrationForm.username->clear();
    showAuthPage(SignInPage);
    setAuthFeedback(signInForm.feedback, "Account created. Sign in with your new password.", true);
    signInForm.password->setFocus();
}
void MainWindow::handleLogout() {
    currentUser.clear();
    learningCentre->setLearner(QString());
    signInForm.username->clear();
    registrationForm.username->clear();
    centralStack->setCurrentIndex(0);
    showAuthPage(SignInPage);
    workspaceHistory.clear();
    showWorkspace(homePage);
}

void MainWindow::runSimulation(){
    bool okAge,okIncome,okLoans,okAsset,okRisk; const std::string id=policyIdInput->text().toStdString(); std::string name=clientNameInput->text().toStdString(); if(name.empty()) name="Thabo Mafokane";
    const int age=ageInput->text().toInt(&okAge), risk=riskParamInput->text().toInt(&okRisk); const double income=incomeInput->text().toDouble(&okIncome),loans=loansInput->text().toDouble(&okLoans),asset=assetInput->text().toDouble(&okAsset);
    const std::array<QPair<QLineEdit *, bool>, 6> checks = {{
        {policyIdInput, !id.empty()}, {ageInput, okAge && age >= 18},
        {incomeInput, okIncome}, {loansInput, okLoans}, {assetInput, okAsset && asset > 0},
        {riskParamInput, okRisk}
    }};
    QLineEdit *firstInvalid = nullptr;
    for (const auto &[input, valid] : checks) {
        input->setProperty("invalid", !valid);
        input->style()->unpolish(input);
        input->style()->polish(input);
        if (!valid && !firstInvalid) firstInvalid = input;
    }
    if (firstInvalid) {
        firstInvalid->setFocus();
        simulationStatus->setText("Check the highlighted fields");
        QMessageBox::warning(this,"Validation Error","Please fill in all fields with valid client details and numeric parameters.");
        return;
    }
    simulationStatus->setText("Running simulation…");
    simProgressBar->setValue(25);QCoreApplication::processEvents();
    Policyholder ph(id,currentUser.isEmpty()?"Admin":currentUser.toStdString(),name,phoneInput->text().toStdString(),addressInput->text().toStdString(),static_cast<CoverType>(coverTypeCombo->currentIndex()),age,income,loans,geoZoneCombo->currentText().toStdString(),asset,risk);
    simProgressBar->setValue(60);
    QCoreApplication::processEvents();
    const double score = RiskEvaluator::calculateRiskScore(ph);
    const double loss = RiskEvaluator::calculateExpectedLoss(ph);
    constexpr int trials = 10000;
    MonteCarloSimulator sim(trials, 4);
    const auto losses = sim.simulateLosses(ph);
    const double var = sim.calculateVaR(losses, .95);
    ActuarialEngine engine(.15, .05);
    const double annual = engine.calculateRiskAdjustedPremium(ph, var);
    const double monthly = annual / 12.;
    simProgressBar->setValue(100);
    simulationContext->setText(QString("%1  ·  %2  ·  %3").arg(
        QString::fromStdString(id), QString::fromStdString(name), coverTypeCombo->currentText()));
    simulationStatus->setText(QString("%1 trials complete").arg(QLocale().toString(trials)));
    riskScoreKpi->findChild<QLabel*>("value");
    const auto values=[&](QFrame *card,const QString &v){for(auto *label:card->findChildren<QLabel*>())if(label->objectName()=="value"){label->setText(v);break;}}; values(riskScoreKpi,QString::number(score,'f',2));values(expectedLossKpi,"M "+QString::number(loss,'f',2));values(varKpi,"M "+QString::number(var,'f',2));values(monthlyPremiumKpi,"M "+QString::number(monthly,'f',2));
    const bool decline = score >= 75, refer = score >= 55;
    decisionBadge->setText(decline ? "× Recommend decline" : refer ? "! Recommend referral" : "✓ Recommend approval");
    decisionBadge->setProperty("decision", decline ? "decline" : refer ? "refer" : "accept");
    decisionBadge->style()->unpolish(decisionBadge);
    decisionBadge->style()->polish(decisionBadge);
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












































