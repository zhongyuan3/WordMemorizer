#ifndef WORDDATABASE_H
#define WORDDATABASE_H

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

struct PracticeEntry {
  QString word;
  QString definition;
  QString userAnswer;
};

enum class PracticeMode {
  WordToDefinition = 0,
  DefinitionToWord,
};

struct PracticeRecord {
  QDateTime practiceTime;
  QTime duration;
  QVector<PracticeEntry> correctWords;
  QVector<PracticeEntry> wrongWords;
  double correctRate = 0.0;
  PracticeMode mode;

  qsizetype wordCount() const {
    return correctWords.size() + wrongWords.size();
  }
};

struct PracticeRecordConstView {
  qsizetype index;
  const PracticeRecord& record;
};

class WordDatabaseError {
 public:
  WordDatabaseError() = default;
  WordDatabaseError(const QString& message) : m_message(message) {}
  WordDatabaseError(const char* message) : m_message(message) {}
  ~WordDatabaseError() = default;

  const QString& message() const { return m_message; }

 private:
  QString m_message;
};

class WordDatabase {
 public:
  WordDatabase() = default;
  WordDatabase(const QString& filename) : m_filename(filename) {}
  WordDatabase(const QString& filename, const QString& practiceRecordsFilename)
      : m_filename(filename),
        m_practiceRecordsFilename(practiceRecordsFilename) {}
  ~WordDatabase() = default;

  void setFilename(const QString& filename) { m_filename = filename; }

  void setPracticeRecordsFilename(const QString& practiceRecordsFilename) {
    m_practiceRecordsFilename = practiceRecordsFilename;
  }

  const QVector<PracticeRecord>& getPracticeRecords() const {
    return m_practiceRecords;
  }
  QVector<PracticeRecord>& getMutPracticeRecords() { return m_practiceRecords; }
  int practiceRecordCount() const { return m_practiceRecords.size(); }

  const QVector<WordEntry>& getWords() const { return m_words; }
  QVector<WordEntry>& getMutWords() { return m_words; }
  int wordCount() const { return m_words.size(); }
  bool containsWord(const QString& word) const;
  QVector<std::tuple<qsizetype, const WordEntry*>> queryWords(
      const QString& query) const;
  std::optional<WordDatabaseError> addWord(const QString& word,
                                           const QString& definition,
                                           int priority);
  std::optional<WordDatabaseError> removeWord(const QString& word);
  std::optional<WordDatabaseError> updateWord(const QString& oldWord,
                                              const QString& newWord,
                                              const QString& newDefinition,
                                              int newPriority);
  std::optional<WordDatabaseError> saveToFile() const;
  std::optional<WordDatabaseError> loadFromFile();

 private:
  std::optional<WordDatabaseError> loadWordsFromFile();
  std::optional<WordDatabaseError> loadPracticeRecordsFromFile();
  std::optional<WordDatabaseError> saveWordsToFile() const;
  std::optional<WordDatabaseError> savePracticeRecordsToFile() const;

  QVector<WordEntry> m_words;
  QString m_filename;
  QVector<PracticeRecord> m_practiceRecords;
  QString m_practiceRecordsFilename;
};

#endif
