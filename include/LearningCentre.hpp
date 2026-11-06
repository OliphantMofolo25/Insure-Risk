#pragma once

#include <QWidget>
#include <array>

class QLabel;
class QProgressBar;
class QPushButton;
class QRadioButton;
class QStackedWidget;
class LessonVisual;

class LearningCentre : public QWidget {
public:
    explicit LearningCentre(QWidget *parent = nullptr);
    void setLearner(const QString &username);
    void setDarkMode(bool dark);

private:
    void openLesson(int index);
    void checkAnswer();
    void refreshProgress();

    int currentLesson = 0;
    QString learnerKey;
    std::array<bool, 3> completed{};
    std::array<QPushButton *, 3> lessonButtons{};
    std::array<QRadioButton *, 3> answers{};
    QLabel *progressText;
    QProgressBar *progress;
    QLabel *lessonTitle;
    QLabel *lessonSummary;
    QLabel *practiceNote;
    QLabel *question;
    QLabel *feedback;
    QStackedWidget *controls;
    LessonVisual *visual;
};
