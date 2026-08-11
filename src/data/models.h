#ifndef MODELS_H
#define MODELS_H

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QVector>

/**
 * @brief 单词条目数据结构
 *
 * definitions 使用 QStringList 存储，解决了旧版本中用逗号分隔字符串
 * 导致释义本身含逗号时出错的问题。
 */
struct WordEntry {
    QString word;
    QStringList definitions;
    QDateTime createdAt;
    QDateTime modifiedAt;
    int priority = 1;
    int totalAttempts = 0;
    int totalErrors = 0;

    /// 将释义列表拼接为逗号分隔的文本（用于显示）
    QString definitionsText() const { return definitions.join(", "); }

    /// 从逗号分隔文本构建释义列表（用于兼容旧数据）
    static QStringList parseDefinitions(const QString &text) {
        return text.split(",", Qt::SkipEmptyParts);
    }
};

/// 练习模式枚举
enum class PracticeMode {
    WordToDefinition = 0,
    DefinitionToWord,
};

/// 单道练习题的作答记录
struct PracticeEntry {
    QString word;
    QString definition;
    QString userAnswer;
};

/// 一次完整练习的记录
struct PracticeRecord {
    QDateTime practiceTime;
    QTime duration;
    QVector<PracticeEntry> correctWords;
    QVector<PracticeEntry> wrongWords;
    double correctRate = 0.0;
    PracticeMode mode = PracticeMode::WordToDefinition;

    qsizetype wordCount() const {
        return correctWords.size() + wrongWords.size();
    }
};

/**
 * @brief 数据库操作错误类型
 *
 * 替代旧版 WordDatabaseError，命名更通用。
 */
class DatabaseError {
public:
    DatabaseError() = default;
    DatabaseError(const QString &message) : m_message(message) {}
    DatabaseError(const char *message) : m_message(message) {}

    const QString &message() const { return m_message; }

private:
    QString m_message;
};

#endif  // MODELS_H
