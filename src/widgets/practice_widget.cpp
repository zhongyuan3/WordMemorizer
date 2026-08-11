#include "widgets/practice_widget.h"

#include <QChar>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>

#include "core/practice_session.h"
#include "widgets/practice_result_delegate.h"

PracticeWidget::PracticeWidget(QWidget *parent) : QWidget(parent) {
    // ---- 顶部：计时器 + 退出按钮 ----
    m_timerLabel = new QLabel("练习时间: 00:00");
    m_timerLabel->setObjectName("m_practiceTimerLabel");

    m_exitButton = new QPushButton("退出练习");
    m_exitButton->setObjectName("m_exitPracticeButton");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setSpacing(10);
    topLayout->addWidget(m_timerLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_exitButton);

    // ---- 题目 ----
    m_questionLabel = new QLabel;
    m_questionLabel->setObjectName("m_questionLabel");
    m_questionLabel->setWordWrap(true);
    m_questionLabel->setAlignment(Qt::AlignCenter);

    // ---- 答案输入 + 提交 ----
    m_answerEdit = new QLineEdit;
    m_answerEdit->setObjectName("m_answerEdit");
    m_answerEdit->setReadOnly(true);
    m_answerEdit->setPlaceholderText("在此输入答案...");

    m_submitButton = new QPushButton("提交");
    m_submitButton->setObjectName("m_submitButton");
    m_submitButton->setEnabled(false);

    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->setSpacing(10);
    inputLayout->addWidget(m_answerEdit);
    inputLayout->addWidget(m_submitButton);

    // ---- 正确率 ----
    m_accuracyLabel = new QLabel;
    m_accuracyLabel->setObjectName("m_practiceAccuracyLabel");

    // ---- 结果列表 ----
    m_resultsModel = new QStandardItemModel(this);

    m_resultsList = new QListView;
    m_resultsList->setObjectName("m_practiceWordsListWidget");
    m_resultsList->setModel(m_resultsModel);

    m_delegate = new PracticeResultDelegate(this);
    m_resultsList->setItemDelegate(m_delegate);

    // ---- 主布局 ----
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_questionLabel);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_accuracyLabel);
    mainLayout->addWidget(m_resultsList);

    // ---- 信号连接 ----
    connect(m_submitButton, &QPushButton::clicked, this,
            &PracticeWidget::onSubmitClicked);
    connect(m_exitButton, &QPushButton::clicked, this,
            &PracticeWidget::onExitClicked);
    connect(m_answerEdit, &QLineEdit::returnPressed, m_submitButton,
            &QPushButton::click);
}

void PracticeWidget::setSession(PracticeSession *session) {
    m_session = session;
    if (!m_session) return;

    connect(m_session, &PracticeSession::questionChanged, this,
            &PracticeWidget::onQuestionChanged);
    connect(m_session, &PracticeSession::answerSubmitted, this,
            &PracticeWidget::onAnswerSubmitted);
    connect(m_session, &PracticeSession::timerUpdated, this,
            &PracticeWidget::onTimerUpdated);
    connect(m_session, &PracticeSession::sessionStarted, this,
            &PracticeWidget::onSessionStarted);
    connect(m_session, &PracticeSession::sessionEnded, this,
            &PracticeWidget::onSessionEnded);
}

void PracticeWidget::reset() {
    m_timerLabel->setText("练习时间: 00:00");
    m_questionLabel->clear();
    m_answerEdit->clear();
    m_answerEdit->setReadOnly(true);
    m_submitButton->setEnabled(false);
    m_accuracyLabel->clear();
    m_resultsModel->clear();
}

void PracticeWidget::onQuestionChanged(const QString &text) {
    m_questionLabel->setText(text);
    m_answerEdit->clear();
    m_submitButton->setEnabled(true);
    m_answerEdit->setReadOnly(false);
    m_answerEdit->setFocus();
}

void PracticeWidget::onAnswerSubmitted(bool correct,
                                       const QString &correctAnswer) {
    m_submitButton->setEnabled(false);
    m_answerEdit->setReadOnly(true);

    // 在结果列表中添加一行，用 UserRole 标记正误（true 表示错误 -> 红色）
    QString word = m_session ? m_session->currentWord() : QString();
    QString text = QString("%1 - %2").arg(word).arg(correctAnswer);

    QStandardItem *item = new QStandardItem(text);
    item->setData(!correct, Qt::UserRole);
    m_resultsModel->appendRow(item);

    // 延迟 1 秒后跳到下一题，给用户时间查看正确答案
    QTimer::singleShot(1000, this, [this]() {
        if (m_session) {
            m_session->nextQuestion();
        }
    });
}

void PracticeWidget::onTimerUpdated(qint64 elapsed) {
    int minutes = static_cast<int>(elapsed / 60);
    int seconds = static_cast<int>(elapsed % 60);
    m_timerLabel->setText(QString("练习时间: %1:%2")
                              .arg(minutes, 2, 10, QChar('0'))
                              .arg(seconds, 2, 10, QChar('0')));
}

void PracticeWidget::onSessionStarted() {
    show();
    m_answerEdit->setReadOnly(false);
    m_answerEdit->setFocus();
}

void PracticeWidget::onSessionEnded() {
    if (!m_session) return;

    int correct = m_session->correctCount();
    int total = m_session->totalQuestions();
    double rate = m_session->correctRate();

    m_accuracyLabel->setText(QString("答对题数: %1 / %2\n正确率: %3%")
                                 .arg(correct)
                                 .arg(total)
                                 .arg(rate, 0, 'f', 2));
}

void PracticeWidget::onSubmitClicked() {
    if (m_session) {
        m_session->submitAnswer(m_answerEdit->text());
    }
}

void PracticeWidget::onExitClicked() { emit exitRequested(); }
