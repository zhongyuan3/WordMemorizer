#ifndef WORDDATABASE_H
#define WORDDATABASE_H

#include <QDateTime>
#include <QString>
#include <QVector>
#include <optional>
#include <unordered_set>

struct WordEntry {
  QString word;
  QString definition;
  QDateTime createdAt;
  QDateTime modifiedAt;
  int priority;
};

using WordDatabaseError = QString;

class WordDatabase {
 public:
  WordDatabase() = default;
  WordDatabase(const QString& filename) : m_filename(filename) {}
  ~WordDatabase() = default;

  const QVector<WordEntry>& getWords() const { return m_words; }
  int wordCount() const { return m_words.size(); }
  bool containsWord(const QString& word) const;
  QVector<std::tuple<qsizetype, const WordEntry*>> queryWords(
      const QString& query) const;
  std::optional<WordDatabaseError> addWord(const QString& word,
                                           const QString& definition,
                                           int priority);
  std::optional<WordDatabaseError> removeWord(const QString& word);
  std::optional<WordDatabaseError> updateWord(const QString& old_word,
                                              const QString& new_word,
                                              const QString& new_definition,
                                              int new_priority);
  std::optional<WordDatabaseError> saveToFile() const;
  std::optional<WordDatabaseError> loadFromFile();

 private:
  QVector<WordEntry> m_words;
  QString m_filename;
};

#endif  // WORDDATABASE_H
