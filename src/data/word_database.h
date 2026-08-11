#ifndef WORD_DATABASE_H
#define WORD_DATABASE_H

#include <QHash>
#include <QString>
#include <QVector>
#include <optional>

#include "data/models.h"

/**
 * @brief 单词与练习记录的数据持久化层
 *
 * 重构要点：
 * - 移除 getMutWords / getMutPracticeRecords，杜绝外部直接修改内部容器
 * - 内部维护 word -> index 的哈希索引，containsWord / removeWord / updateWord
 *   从 O(n) 优化为 O(1) 查找
 * - 所有修改操作通过明确的公共接口完成，保证索引一致性
 */
class WordDatabase {
public:
    WordDatabase() = default;
    WordDatabase(const QString &wordsFilename,
                 const QString &practiceRecordsFilename)
        : m_wordsFilename(wordsFilename),
          m_practiceRecordsFilename(practiceRecordsFilename) {}

    void setWordsFilename(const QString &filename) {
        m_wordsFilename = filename;
    }

    void setPracticeRecordsFilename(const QString &filename) {
        m_practiceRecordsFilename = filename;
    }

    // ---- 查询接口（只读） ----
    const QVector<WordEntry> &words() const { return m_words; }
    const QVector<PracticeRecord> &practiceRecords() const {
        return m_practiceRecords;
    }

    int wordCount() const { return m_words.size(); }
    int practiceRecordCount() const { return m_practiceRecords.size(); }

    bool containsWord(const QString &word) const;
    const WordEntry *findWord(const QString &word) const;

    // ---- 修改接口 ----
    std::optional<DatabaseError> addWord(const QString &word,
                                         const QStringList &definitions,
                                         int priority);
    std::optional<DatabaseError> removeWord(const QString &word);
    std::optional<DatabaseError> updateWord(const QString &oldWord,
                                            const QString &newWord,
                                            const QStringList &newDefinitions,
                                            int newPriority);

    /// 递增某单词的练习统计（由 PracticeSession 调用）
    void recordAttempt(const QString &word, bool wasCorrect);

    /// 追加一条练习记录
    void addPracticeRecord(const PracticeRecord &record);

    // ---- 持久化 ----
    std::optional<DatabaseError> saveToFile() const;
    std::optional<DatabaseError> loadFromFile();

private:
    std::optional<DatabaseError> loadWordsFromFile();
    std::optional<DatabaseError> loadPracticeRecordsFromFile();
    std::optional<DatabaseError> saveWordsToFile() const;
    std::optional<DatabaseError> savePracticeRecordsToFile() const;

    void rebuildIndex();

    QVector<WordEntry> m_words;
    QHash<QString, qsizetype> m_wordIndex;  // word -> index

    QVector<PracticeRecord> m_practiceRecords;

    QString m_wordsFilename;
    QString m_practiceRecordsFilename;
};

#endif  // WORD_DATABASE_H
