#include "core/practice_session.h"

#include <QDateTime>

#include "core/word_selection.h"

PracticeSession::PracticeSession(WordDatabase *db, QObject *parent)
    : QObject(parent), m_db(db) {}

bool PracticeSession::start(PracticeMode mode, int count) {
    if (m_active) {
        emit error("练习正在进行中");
        return false;
    }

    int totalWords = m_db->wordCount();
    if (totalWords == 0) {
        emit error("还没有录入任何单词");
        return false;
    }
    if (count > totalWords) {
        emit error(QString("请求的题目数量 (%1) 超过了单词总数 (%2)")
                       .arg(count)
                       .arg(totalWords));
        return false;
    }

    m_mode = mode;
    m_totalQuestions = count;
    m_currentIndex = 0;
    m_correctCount = 0;
    m_elapsedSeconds = 0;
    m_active = true;
    m_answerSubmitted = false;
    m_lastCorrect = false;

    m_wordIndices = std::move(randomlySelectWords(m_db->words(), count));

    m_record = PracticeRecord{};
    m_record.practiceTime = QDateTime::currentDateTime();
    m_record.mode = m_mode;

    emit sessionStarted();
    emit timerUpdated(m_elapsedSeconds);

    loadQuestion();
    return true;
}

void PracticeSession::loadQuestion() {
    if (m_currentIndex >= m_totalQuestions) {
        endSession();
        return;
    }

    m_answerSubmitted = false;

    qsizetype wordIdx = m_wordIndices[m_currentIndex];
    const WordEntry &entry = m_db->words()[wordIdx];

    m_currentWord = entry.word;

    if (m_mode == PracticeMode::WordToDefinition) {
        // 给出单词，要求填写释义
        m_currentQuestionText =
            QString("第 %1 题：%2").arg(m_currentIndex + 1).arg(entry.word);
        m_currentAnswerText = entry.definitionsText();
    } else {
        // 给出释义，要求填写单词
        QString def = randomlySelectDefinition(entry);
        m_currentQuestionText =
            QString("第 %1 题：%2").arg(m_currentIndex + 1).arg(def);
        m_currentAnswerText = entry.word;
    }

    emit questionChanged(m_currentQuestionText);
}

void PracticeSession::submitAnswer(const QString &userAnswer) {
    if (!m_active || m_answerSubmitted) return;

    m_answerSubmitted = true;
    QString answer = userAnswer.trimmed();
    bool correct = checkAnswer(answer);
    m_lastCorrect = correct;

    qsizetype wordIdx = m_wordIndices[m_currentIndex];
    const WordEntry &entry = m_db->words()[wordIdx];

    // 记录到练习记录
    PracticeEntry pe;
    pe.word = entry.word;
    pe.definition = entry.definitionsText();
    pe.userAnswer = answer;

    if (correct) {
        m_correctCount++;
        m_record.correctWords.append(pe);
    } else {
        m_record.wrongWords.append(pe);
    }

    // 更新数据库中的练习统计
    m_db->recordAttempt(entry.word, correct);

    emit answerSubmitted(correct, m_currentAnswerText);
}

void PracticeSession::nextQuestion() {
    if (!m_active || !m_answerSubmitted) return;

    m_currentIndex++;
    if (m_currentIndex < m_totalQuestions) {
        loadQuestion();
    } else {
        endSession();
    }
}

void PracticeSession::abort() {
    if (!m_active) return;
    endSession();
}

void PracticeSession::tick() {
    if (!m_active) return;
    m_elapsedSeconds++;
    emit timerUpdated(m_elapsedSeconds);
}

double PracticeSession::correctRate() const {
    if (m_totalQuestions == 0) return 0.0;
    return static_cast<double>(m_correctCount) / m_totalQuestions * 100.0;
}

QString PracticeSession::currentQuestionText() const {
    return m_currentQuestionText;
}

QString PracticeSession::currentAnswerText() const {
    return m_currentAnswerText;
}

QString PracticeSession::currentWord() const { return m_currentWord; }

void PracticeSession::endSession() {
    if (!m_active) return;
    m_active = false;

    m_record.duration = QTime(0, 0, 0).addSecs(m_elapsedSeconds);
    m_record.correctRate = correctRate();

    m_db->addPracticeRecord(m_record);

    emit sessionEnded();
}

bool PracticeSession::checkAnswer(const QString &userAnswer) const {
    if (userAnswer.isEmpty()) return false;

    qsizetype wordIdx = m_wordIndices[m_currentIndex];
    const WordEntry &entry = m_db->words()[wordIdx];

    if (m_mode == PracticeMode::WordToDefinition) {
        // 用户应填写释义，检查是否匹配任意一条释义
        for (const QString &def : entry.definitions) {
            if (def.contains(userAnswer, Qt::CaseInsensitive)) return true;
        }
        return false;
    } else {
        // 用户应填写单词
        return entry.word.contains(userAnswer, Qt::CaseInsensitive);
    }
}
