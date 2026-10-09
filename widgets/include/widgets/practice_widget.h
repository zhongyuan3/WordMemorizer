#ifndef PRACTICE_WIDGET_H
#define PRACTICE_WIDGET_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QListView;
class QPushButton;
class QStandardItemModel;
class PracticeSession;

/**
 * @brief 练习界面 Widget
 *
 * 重构要点：将练习 UI 从 WordMemorizer God Object 中抽离为独立组件。
 * 通过 setSession() 绑定 PracticeSession，监听其信号驱动 UI 更新，
 * 实现业务逻辑与视图的彻底解耦。
 */
class PracticeWidget : public QWidget {
    Q_OBJECT

public:
    explicit PracticeWidget(QWidget *parent = nullptr);

    /// 绑定练习会话，连接信号/槽
    void setSession(PracticeSession *session);

    /// 重置 UI 状态（清空题目、答案、结果列表等）
    void reset();

signals:
    /// 用户点击退出练习时发出
    void exitRequested();

private slots:
    void onQuestionChanged(const QString &text);
    void onAnswerSubmitted(bool correct, const QString &correctAnswer);
    void onTimerUpdated(qint64 elapsed);
    void onSessionStarted();
    void onSessionEnded();
    void onSubmitClicked();
    void onExitClicked();

private:
    QLabel *m_timerLabel;
    QLabel *m_questionLabel;
    QLineEdit *m_answerEdit;
    QPushButton *m_submitButton;
    QPushButton *m_exitButton;
    QLabel *m_accuracyLabel;
    QListView *m_resultsList;
    QStandardItemModel *m_resultsModel;
    PracticeSession *m_session = nullptr;
};

#endif  // PRACTICE_WIDGET_H
