#ifndef PRACTICE_SESSION_H
#define PRACTICE_SESSION_H

#include <QObject>
#include <QString>
#include <QTime>
#include <QVector>

#include "data/models.h"
#include "data/word_database.h"

/**
 * @brief 练习会话状态机
 *
 * 重构要点：
 * - 将练习流程逻辑从 WordMemorizer God Object 中抽离
 * - 统一管理练习状态（题目索引、计时、正确率统计）
 * - 通过信号通知 UI 更新，彻底解耦业务逻辑和视图
 * - 消除 startModeWordToDefinition / startModeDefinitionToWord 的代码重复
 */
class PracticeSession : public QObject {
    Q_OBJECT

public:
    explicit PracticeSession(WordDatabase *db, QObject *parent = nullptr);

    /**
     * @brief 启动一次练习
     * @param mode    练习模式
     * @param count   题目数量
     * @return 是否成功启动（false 时通过 error 信号报告原因）
     */
    bool start(PracticeMode mode, int count);

    /// 提交当前题目的答案
    void submitAnswer(const QString &userAnswer);

    /// 跳到下一题（由 UI 在展示结果后调用）
    void nextQuestion();

    /// 提前终止练习
    void abort();

    // ---- 状态查询 ----
    bool isActive() const { return m_active; }
    int currentIndex() const { return m_currentIndex; }
    int totalQuestions() const { return m_totalQuestions; }
    int correctCount() const { return m_correctCount; }
    qint64 elapsedSeconds() const { return m_elapsedSeconds; }
    PracticeMode mode() const { return m_mode; }
    double correctRate() const;

    /// 获取当前题目的展示文本（题目内容）
    QString currentQuestionText() const;

    /// 获取当前题目的正确答案文本
    QString currentAnswerText() const;

    /// 获取当前题目对应的单词
    QString currentWord() const;

    /// 是否已提交当前题目的答案（等待 nextQuestion）
    bool currentAnswerSubmitted() const { return m_answerSubmitted; }

    /// 上一次提交是否回答正确
    bool lastAnswerCorrect() const { return m_lastCorrect; }

    /// 获取当前练习会话的完整记录（练习结束时使用）
    const PracticeRecord &record() const { return m_record; }

public slots:
    /// 计时器每秒触发
    void tick();

signals:
    void sessionStarted();
    void sessionEnded();
    void questionChanged(const QString &questionText);
    void answerSubmitted(bool correct, const QString &correctAnswer);
    void timerUpdated(qint64 elapsedSeconds);
    void error(const QString &message);

private:
    void loadQuestion();
    void endSession();
    bool checkAnswer(const QString &userAnswer) const;

    WordDatabase *m_db;

    bool m_active = false;
    bool m_answerSubmitted = false;
    bool m_lastCorrect = false;

    int m_currentIndex = -1;
    int m_totalQuestions = 0;
    int m_correctCount = 0;
    qint64 m_elapsedSeconds = 0;
    PracticeMode m_mode = PracticeMode::WordToDefinition;

    QVector<qsizetype> m_wordIndices;
    QString m_currentQuestionText;
    QString m_currentAnswerText;
    QString m_currentWord;

    PracticeRecord m_record;
};

#endif  // PRACTICE_SESSION_H
