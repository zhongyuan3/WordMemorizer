#ifndef WORDDATABASE_HPP
#define WORDDATABASE_HPP

#include <QDateTime>
#include <QString>
#include <QVector>
#include <optional>

struct WordEntry {
    QString word;
    QString definition;
    QDateTime createdAt;
    QDateTime modifiedAt;
    int priority = 1;
    int totalAttempts = 0;
    int totalErrors = 0;
};

class WordDatabaseError {
public:
    WordDatabaseError() = default;
    WordDatabaseError(const QString &message) : m_message(message) {
    }
    ~WordDatabaseError() = default;

    const QString &message() const {
        return m_message;
    }

private:
    QString m_message;
};

class WordDatabase {
public:
    WordDatabase() = default;
    WordDatabase(const QString &filename) : m_filename(filename) {
    }
    ~WordDatabase() = default;

    const QVector<WordEntry> &getWords() const {
        return m_words;
    }
    QVector<WordEntry> &getMutWords() {
        return m_words;
    }
    int wordCount() const {
        return m_words.size();
    }
    bool containsWord(const QString &word) const;
    QVector<std::tuple<qsizetype, const WordEntry *>> queryWords(const QString &query) const;
    std::optional<WordDatabaseError> addWord(const QString &word, const QString &definition, int priority);
    std::optional<WordDatabaseError> removeWord(const QString &word);
    std::optional<WordDatabaseError> updateWord(const QString &oldWord, const QString &newWord,
                                                const QString &newDefinition, int newPriority);
    std::optional<WordDatabaseError> saveToFile() const;
    std::optional<WordDatabaseError> loadFromFile();

private:
    QVector<WordEntry> m_words;
    QString m_filename;
};

#endif  // WORDDATABASE_H
