#include "LearningCentre.hpp"
#include "DynamicPricingEngine.hpp"
#include "MonteCarloSimulator.hpp"
#include "RiskEvaluator.hpp"

#include <QButtonGroup>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSettings>
#include <QSlider>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace {
enum LessonIndex { Application, Risk, Quotes };

struct Lesson {
    const char *title;
    const char *summary;
    const char *question;
    std::array<const char *, 3> answers;
    int correct;
    const char *explanation;
};

constexpr std::array<Lesson, 3> lessons{{
    {"Complete an application",
     "Follow a fictional property application from the policy reference to the inputs used in its risk assessment.",
     "What should you type when the field shows the example ‘2 – Secure’?",
     {"The word Secure", "The whole number 2", "The complete example label"}, 1,
     "Enter 2. The field accepts a whole number. Use your team's rating guidance when assessing a real case."},
    {"Read a risk result",
     "Change a training input and see how it shifts the score and the spread of possible annual losses.",
     "What does VaR at 95% mark in this chart?",
     {"The expected loss", "The largest simulated loss", "The loss threshold at the 95th percentile"}, 2,
     "About 95% of the modeled losses are at or below that threshold. The remaining tail can contain larger losses."},
    {"Compare quotes and deductibles",
     "Compare the premium paid for cover with the deductible the client contributes to an eligible claim.",
     "In this exercise, what does the insurer pay on an eligible M 8,000 loss with a M 2,000 deductible?",
     {"M 6,000", "M 8,000", "M 2,000"}, 0,
     "M 8,000 minus M 2,000 leaves M 6,000 in this simplified case. Real payouts depend on policy terms and limits."}
}};

constexpr int trainingLoss = 8000;
constexpr std::array<int, 3> deductibles{500, 1000, 2000};
constexpr std::array<int, 3> examplePremiums{310, 275, 240};

Policyholder propertyCase(int hazard) {
    return Policyholder("TRAIN-001", "training", "Sample Property Owner", "", "", CoverType::Property,
                        40, 8000, 5000, "Urban", 100000, hazard);
}

QLabel *label(const QString &text, const char *name = "") {
    auto *widget = new QLabel(text);
    widget->setObjectName(name);
    widget->setTextFormat(Qt::PlainText);
    widget->setWordWrap(true);
    return widget;
}

QSettings trainingSettings() {
    return QSettings("Matseliso", "InsureRiskTraining");
}
}

class LessonVisual : public QWidget {
public:
    LessonVisual() {
        setMinimumSize(330, 200);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    }

    void showApplication(int step) {
        lesson = Application;
        selected = step;
        setAccessibleName("Application steps: identify the case, choose the cover, and enter risk details");
        update();
    }

    void showRisk(int hazard) {
        lesson = Risk;
        const auto sample = propertyCase(hazard);
        expectedLoss = RiskEvaluator::calculateExpectedLoss(sample);
        riskScore = RiskEvaluator::calculateRiskScore(sample);
        constexpr int sampleSize = 2400;
        MonteCarloSimulator simulator(sampleSize, 2);
        const auto baseline = propertyCase(2);
        if (baselineLosses.empty()) baselineLosses = simulator.simulateLosses(baseline);
        // Hold the sampled percentiles constant so the slider isolates the effect of the risk input.
        const double multiplier = expectedLoss / RiskEvaluator::calculateExpectedLoss(baseline);
        axisMaximum = 4 * RiskEvaluator::calculateExpectedLoss(propertyCase(5));
        auto losses = baselineLosses;
        bins.fill(0);
        for (double &loss : losses) {
            loss *= multiplier;
            const auto bin = static_cast<size_t>(loss / axisMaximum * bins.size());
            ++bins[std::min(bin, bins.size() - 1)];
        }
        var = simulator.calculateVaR(losses, .95);
        setAccessibleName(QString("Loss histogram. Expected loss %1. Value at risk %2.")
                          .arg(DynamicPricingEngine::money(expectedLoss), DynamicPricingEngine::money(var)));
        update();
    }

    void showQuote(int index) {
        lesson = Quotes;
        selected = index;
        setAccessibleName("Eligible loss split into the deductible and the modeled insurer payment");
        update();
    }

    void setDarkMode(bool value) { dark = value; update(); }

    QString riskSummary() const {
        return QString("Risk score %1 / 100\nExpected loss %2\nVaR 95% %3")
            .arg(QString::number(riskScore, 'f', 1), DynamicPricingEngine::money(expectedLoss),
                 DynamicPricingEngine::money(var));
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const QColor surface(dark ? "#102e2a" : "#eaf8f0");
        const QColor ink(dark ? "#ecfdf5" : "#12372a");
        const QColor accent(dark ? "#86efac" : "#166534");
        const QColor amber(dark ? "#fbbf24" : "#92400e");
        painter.setPen(Qt::NoPen);
        painter.setBrush(surface);
        painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 14, 14);
        painter.setPen(ink);
        QFont title = font();
        title.setPixelSize(13);
        title.setBold(true);
        painter.setFont(title);

        if (lesson == Application) {
            painter.drawText(QRectF(18, 14, width() - 36, 22), "FROM FORM TO ASSESSMENT");
            const std::array<QString, 3> names{"Policy\nreference", "Cover\ntype", "Risk\ninputs"};
            const double stepWidth = (width() - 64.0) / 3.0;
            const double y = height() / 2.0 - 22;
            painter.setPen(QPen(accent, 2));
            painter.drawLine(QPointF(32, y + 24), QPointF(width() - 32, y + 24));
            for (int i = 0; i < 3; ++i) {
                const QRectF tile(18 + i * (stepWidth + 14), y - 12, stepWidth, 90);
                painter.setPen(QPen(accent, i == selected ? 2 : 1));
                painter.setBrush(i == selected ? accent : surface);
                painter.drawRoundedRect(tile, 10, 10);
                painter.setPen(i == selected && !dark ? Qt::white : i == selected ? QColor("#12372a") : ink);
                painter.drawText(tile.adjusted(3, 7, -3, -55), Qt::AlignCenter, QString("0%1").arg(i + 1));
                painter.drawText(tile.adjusted(3, 32, -3, -5), Qt::AlignCenter, names[i]);
            }
        } else if (lesson == Risk) {
            painter.drawText(QRectF(18, 14, width() - 36, 22), "SIMULATED ANNUAL LOSSES");
            const QRectF graph(21, 52, width() - 42, height() - 91);
            const int maxBin = *std::max_element(bins.cbegin(), bins.cend());
            const double barWidth = graph.width() / bins.size();
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(dark ? "#3d9a75" : "#48a579"));
            for (size_t i = 0; i < bins.size(); ++i) {
                const double barHeight = maxBin ? graph.height() * bins[i] / maxBin : 0;
                painter.drawRect(QRectF(graph.left() + i * barWidth, graph.bottom() - barHeight,
                                       std::max(1.0, barWidth - 3), barHeight));
            }
            painter.setPen(QPen(ink, 1));
            painter.drawLine(graph.bottomLeft(), graph.bottomRight());
            const auto marker = [&](double value, const QColor &colour, const QString &caption) {
                const double x = graph.left() + graph.width() * std::min(1.0, value / axisMaximum);
                painter.setPen(QPen(colour, 2, Qt::DashLine));
                painter.drawLine(QPointF(x, graph.top()), QPointF(x, graph.bottom()));
                painter.setPen(colour);
                painter.drawText(QRectF(x - 44, graph.top() - 21, 88, 19), Qt::AlignCenter, caption);
            };
            marker(expectedLoss, ink, "Mean");
            marker(var, amber, "VaR 95%");
            painter.setPen(ink);
            painter.drawText(QRectF(21, graph.bottom() + 8, 50, 18), "M 0");
            painter.drawText(QRectF(78, graph.bottom() + 8, width() - 99, 18), Qt::AlignRight,
                             DynamicPricingEngine::money(axisMaximum) + "+");
        } else {
            painter.drawText(QRectF(18, 14, width() - 36, 22), "HOW THE LOSS IS SHARED");
            const QRectF bar(20, height() / 2.0 - 15, width() - 40, 38);
            const double deductibleWidth = bar.width() * deductibles[selected] / trainingLoss;
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#16804a"));
            painter.drawRoundedRect(bar, 9, 9);
            painter.setBrush(QColor("#eab84d"));
            painter.drawRoundedRect(QRectF(bar.x(), bar.y(), deductibleWidth, bar.height()), 8, 8);
            painter.setPen(ink);
            painter.drawText(QRectF(20, bar.top() - 27, bar.width(), 22), "M 8,000 eligible loss");
            painter.setPen(amber);
            painter.drawText(QRectF(20, bar.bottom() + 11, bar.width() / 2, 20), "Client deductible");
            painter.setPen(accent);
            painter.drawText(QRectF(20 + bar.width() / 2, bar.bottom() + 11, bar.width() / 2, 20),
                             Qt::AlignRight, "Insurer payment");
        }
    }

private:
    int lesson = Application;
    int selected = 0;
    bool dark = false;
    double expectedLoss = 0, var = 0, riskScore = 0, axisMaximum = 1;
    std::array<int, 16> bins{};
    std::vector<double> baselineLosses;
};

LearningCentre::LearningCentre(QWidget *parent) : QWidget(parent) {
    setObjectName("learningCentre");
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 6, 0, 10);
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    root->addWidget(scroll);
    auto *page = new QWidget;
    page->setObjectName("learningPage");
    scroll->setWidget(page);
    auto *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(12, 8, 12, 12);
    pageLayout->setSpacing(16);

    auto *banner = new QFrame;
    banner->setObjectName("learningHero");
    auto *hero = new QHBoxLayout(banner);
    hero->setContentsMargins(24, 18, 24, 18);
    auto *intro = new QVBoxLayout;
    intro->addWidget(label("OFFICER TRAINING  ·  PRACTICE MODE", "learningEyebrow"));
    intro->addWidget(label("Learning Centre", "learningTitle"));
    intro->addWidget(label("Practice an insurance decision with a fictional case. Training stays separate from policy records."));
    hero->addLayout(intro, 2);
    auto *progressBox = new QVBoxLayout;
    progressText = label("", "learningEyebrow");
    progressBox->addWidget(progressText);
    progress = new QProgressBar;
    progress->setRange(0, static_cast<int>(lessons.size()));
    progress->setTextVisible(false);
    progress->setAccessibleName("Training completion");
    progressBox->addWidget(progress);
    hero->addLayout(progressBox, 1);
    pageLayout->addWidget(banner);

    auto *content = new QHBoxLayout;
    content->setSpacing(14);
    auto *navigation = new QFrame;
    navigation->setObjectName("learningCard");
    navigation->setFixedWidth(193);
    auto *nav = new QVBoxLayout(navigation);
    nav->setContentsMargins(13, 14, 13, 14);
    nav->addWidget(label("YOUR LESSONS", "learningEyebrow"));
    auto *lessonGroup = new QButtonGroup(this);
    for (int i = 0; i < static_cast<int>(lessons.size()); ++i) {
        auto *button = new QPushButton;
        button->setObjectName("lessonButton");
        button->setCheckable(true);
        button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        lessonGroup->addButton(button, i);
        nav->addWidget(button);
        lessonButtons[i] = button;
    }
    nav->addStretch();
    nav->addWidget(label("Start anywhere. A correct quiz answer marks the lesson complete.", "learningHint"));
    content->addWidget(navigation);
    connect(lessonGroup, &QButtonGroup::idClicked, this, &LearningCentre::openLesson);

    auto *body = new QVBoxLayout;
    auto *topic = new QFrame;
    topic->setObjectName("learningCard");
    auto *topicLayout = new QVBoxLayout(topic);
    topicLayout->setContentsMargins(18, 14, 18, 16);
    lessonTitle = label("", "learningHeading");
    lessonSummary = label("", "learningHint");
    topicLayout->addWidget(lessonTitle);
    topicLayout->addWidget(lessonSummary);
    body->addWidget(topic);

    auto *panels = new QHBoxLayout;
    auto *practice = new QFrame;
    practice->setObjectName("learningCard");
    auto *practiceLayout = new QVBoxLayout(practice);
    practiceLayout->setContentsMargins(16, 14, 16, 14);
    practiceLayout->addWidget(label("TRY IT", "learningEyebrow"));
    visual = new LessonVisual;
    practiceLayout->addWidget(visual, 1);
    controls = new QStackedWidget;
    auto *applicationSteps = new QComboBox;
    applicationSteps->setAccessibleName("Application step");
    applicationSteps->addItems({"1. Policy reference", "2. Cover type", "3. Hazard / Security"});
    controls->addWidget(applicationSteps);
    auto *hazardRow = new QWidget;
    auto *hazardLayout = new QHBoxLayout(hazardRow);
    hazardLayout->setContentsMargins(0, 0, 0, 0);
    auto *hazard = new QSlider(Qt::Horizontal);
    hazard->setRange(1, 5);
    hazard->setValue(2);
    hazard->setAccessibleName("Training hazard rating");
    auto *hazardValue = label("2", "learningEyebrow");
    hazardLayout->addWidget(label("Training hazard rating"));
    hazardLayout->addWidget(hazard, 1);
    hazardLayout->addWidget(hazardValue);
    controls->addWidget(hazardRow);
    auto *deductible = new QComboBox;
    deductible->setAccessibleName("Practice deductible");
    for (size_t i = 0; i < deductibles.size(); ++i) {
        deductible->addItem(QString("Option %1 · %2 deductible")
            .arg(i + 1).arg(DynamicPricingEngine::money(deductibles[i])));
    }
    controls->addWidget(deductible);
    practiceLayout->addWidget(controls);
    practiceNote = label("", "learningHint");
    practiceNote->setMinimumHeight(54);
    practiceLayout->addWidget(practiceNote);
    panels->addWidget(practice, 3);

    auto *quiz = new QFrame;
    quiz->setObjectName("learningCard");
    quiz->setMinimumWidth(245);
    auto *quizLayout = new QVBoxLayout(quiz);
    quizLayout->setContentsMargins(16, 14, 16, 14);
    quizLayout->addWidget(label("KNOWLEDGE CHECK", "learningEyebrow"));
    question = label("", "learningQuestion");
    quizLayout->addWidget(question);
    for (auto &answer : answers) {
        answer = new QRadioButton;
        quizLayout->addWidget(answer);
    }
    auto *submit = new QPushButton("Check answer");
    submit->setObjectName("GreenPrimaryBtn");
    quizLayout->addWidget(submit);
    feedback = label("Choose one answer to get feedback.", "learningHint");
    quizLayout->addWidget(feedback);
    quizLayout->addStretch();
    panels->addWidget(quiz, 2);
    body->addLayout(panels, 1);
    content->addLayout(body, 1);
    pageLayout->addLayout(content, 1);

    const auto applicationPractice = [this](int step) {
        static const std::array<QString, 3> details{
            "TRAIN-001 is a fictional policy reference. Use the case reference to match the application to its records.",
            "Property Insurance is the cover in this example. Confirm the cover type with the application details.",
            "Type a whole number such as 2. The example label says Secure. Confirm real ratings using your team's guidance."
        };
        visual->showApplication(step);
        practiceNote->setText(details[step]);
    };
    const auto riskPractice = [this, hazardValue](int value) {
        hazardValue->setText(QString::number(value));
        visual->showRisk(value);
        practiceNote->setText(visual->riskSummary());
    };
    const auto quotePractice = [this](int option) {
        visual->showQuote(option);
        practiceNote->setText(QString("Example monthly premium %1\nClient deductible %2\nModeled insurer payment %3")
            .arg(DynamicPricingEngine::money(examplePremiums[option]),
                 DynamicPricingEngine::money(deductibles[option]),
                 DynamicPricingEngine::money(trainingLoss - deductibles[option])));
    };
    connect(applicationSteps, &QComboBox::currentIndexChanged, this, applicationPractice);
    connect(hazard, &QSlider::valueChanged, this, riskPractice);
    connect(deductible, &QComboBox::currentIndexChanged, this, quotePractice);
    connect(controls, &QStackedWidget::currentChanged, this,
            [=](int index) {
        if (index == Application) applicationPractice(applicationSteps->currentIndex());
        else if (index == Risk) riskPractice(hazard->value());
        else quotePractice(deductible->currentIndex());
    });
    connect(submit, &QPushButton::clicked, this, &LearningCentre::checkAnswer);

    setDarkMode(false);
    refreshProgress();
    openLesson(Application);
    applicationPractice(0);
}

void LearningCentre::setLearner(const QString &username) {
    const auto key = QString::fromLatin1(username.toUtf8().toHex());
    if (key == learnerKey) return;
    learnerKey = key;
    auto settings = trainingSettings();
    for (int i = 0; i < static_cast<int>(lessons.size()); ++i) {
        completed[i] = !learnerKey.isEmpty() &&
            settings.value(learnerKey + "/lesson-" + QString::number(i), false).toBool();
    }
    refreshProgress();
    openLesson(Application);
}

void LearningCentre::openLesson(int index) {
    currentLesson = index;
    lessonButtons[index]->setChecked(true);
    const auto &lesson = lessons[index];
    lessonTitle->setText(lesson.title);
    lessonSummary->setText(lesson.summary);
    controls->setCurrentIndex(index);
    question->setText(lesson.question);
    for (int i = 0; i < static_cast<int>(answers.size()); ++i) {
        answers[i]->setAutoExclusive(false);
        answers[i]->setChecked(false);
        answers[i]->setAutoExclusive(true);
        answers[i]->setText(lesson.answers[i]);
    }
    feedback->setText(completed[index] ? "Lesson complete. You can try it again." : "Choose one answer to get feedback.");
}

void LearningCentre::checkAnswer() {
    const auto &lesson = lessons[currentLesson];
    if (answers[lesson.correct]->isChecked()) {
        completed[currentLesson] = true;
        if (!learnerKey.isEmpty()) {
            auto settings = trainingSettings();
            settings.setValue(learnerKey + "/lesson-" + QString::number(currentLesson), true);
        }
        refreshProgress();
        feedback->setText(QString("✓ Correct. %1").arg(lesson.explanation));
    } else if (std::any_of(answers.cbegin(), answers.cend(), [](auto *answer) { return answer->isChecked(); })) {
        feedback->setText("Try again. Use the practice panel to check your reasoning.");
    } else {
        feedback->setText("Choose an answer first.");
    }
}

void LearningCentre::refreshProgress() {
    const auto total = std::count(completed.cbegin(), completed.cend(), true);
    progress->setValue(static_cast<int>(total));
    progressText->setText(QString("%1 OF %2 LESSONS COMPLETE").arg(total).arg(lessons.size()));
    const std::array<QString, 3> names{"Application basics", "Risk simulation", "Quote comparison"};
    for (int i = 0; i < static_cast<int>(lessonButtons.size()); ++i) {
        lessonButtons[i]->setText(QString("%1   %2").arg(completed[i] ? QString("✓") : QString::number(i + 1), names[i]));
        lessonButtons[i]->setAccessibleName(names[i] + (completed[i] ? ", complete" : ", incomplete"));
    }
}

void LearningCentre::setDarkMode(bool dark) {
    visual->setDarkMode(dark);
    const QString light = R"(
        QWidget#learningPage, QStackedWidget { background:transparent; }
        QFrame#learningHero { background:#14532d; border-radius:16px; }
        QFrame#learningHero QLabel { color:#ecfdf5; }
        QLabel#learningTitle { font-size:30px; font-weight:700; }
        QLabel#learningHeading { font-size:22px; font-weight:700; }
        QLabel#learningEyebrow { font-size:11px; font-weight:700; }
        QLabel#learningQuestion { font-size:15px; font-weight:700; }
        QLabel#learningHint { color:#475569; }
        QFrame#learningCard { background:#ffffff; border:1px solid #cbd5e1; border-radius:14px; }
        QPushButton#lessonButton { text-align:left; padding:9px 7px; }
        QPushButton#lessonButton:checked { background:#dcfce7; color:#14532d; border-color:#15803d; }
        QRadioButton { spacing:8px; padding:4px 0; }
    )";
    setStyleSheet(dark ? light + R"(
        QFrame#learningCard { background:#172338; border-color:#334155; }
        QLabel#learningHint { color:#cbd5e1; }
        QPushButton#lessonButton:checked { background:#14532d; color:#dcfce7; border-color:#4ade80; }
    )" : light);
}
