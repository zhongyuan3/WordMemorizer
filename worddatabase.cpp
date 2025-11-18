#include "worddatabase.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

std::optional<WordDatabaseError> WordDatabase::addWord(
    const QString& word, const QString& definition, int priority) {
  if (word.isEmpty()) {
    return "单词不能为空";
  }

  if (definition.isEmpty()) {
    return "定义不能为空";
  }

  if (containsWord(word)) {
    return "单词已存在";
  }

  QDateTime now = QDateTime::currentDateTime();

  m_words.append({word.trimmed(), definition.trimmed(), now, now, priority});

  return std::nullopt;
}

bool WordDatabase::containsWord(const QString& word) const {
  for (const auto& entry : m_words) {
    if (entry.word == word) {
      return true;
    }
  }

  return false;
}

QVector<std::tuple<qsizetype, const WordEntry*>> WordDatabase::queryWords(
    const QString& query) const {
  QVector<std::tuple<qsizetype, const WordEntry*>> results;
  for (qsizetype i = 0; i < m_words.size(); ++i) {
    const WordEntry* entry = &m_words[i];
    if (entry->word.contains(query)) {
      results.append({i, entry});
    }
  }
  return results;
}

std::optional<WordDatabaseError> WordDatabase::removeWord(const QString& word) {
  for (qsizetype i = 0; i < m_words.size(); ++i) {
    if (m_words[i].word == word) {
      m_words.removeAt(i);
      return std::nullopt;
    }
  }
  return "单词不存在";
}

std::optional<WordDatabaseError> WordDatabase::saveToFile() const {
  auto err = saveWordsToFile();
  if (err) {
    return err;
  }

  err = savePracticeRecordsToFile();
  if (err) {
    return err;
  }

  return std::nullopt;
}

std::optional<WordDatabaseError> WordDatabase::loadFromFile() {
  auto err = loadWordsFromFile();
  if (err.has_value()) {
    return err;
  }

  err = loadPracticeRecordsFromFile();
  if (err.has_value()) {
    return err;
  }

  return std::nullopt;
}

static QJsonArray practiceEntriesSerialize(
    const QVector<PracticeEntry>& practiceEntries) {
  QJsonArray array;
  for (const auto& entry : practiceEntries) {
    QJsonObject obj;
    obj.insert("word", entry.word);
    obj.insert("definition", entry.definition);
    obj.insert("userAnswer", entry.userAnswer);
    array.append(obj);
  }
  return array;
}

static QJsonObject practiceRecordSerialize(const PracticeRecord& record) {
  QJsonObject obj;
  obj.insert("practiceTime", record.practiceTime.toString(Qt::ISODate));
  obj.insert("duration", record.duration.toString("hh:mm:ss"));
  obj.insert("correctWords", practiceEntriesSerialize(record.correctWords));
  obj.insert("wrongWords", practiceEntriesSerialize(record.wrongWords));
  obj.insert("correctRate", record.correctRate);
  obj.insert("mode", static_cast<int>(record.mode));
  return obj;
}

static QJsonArray practiceRecordsSerialize(
    const QVector<PracticeRecord>& records) {
  QJsonArray array{};
  for (const auto& record : records) {
    array.append(practiceRecordSerialize(record));
  }
  return array;
}

static QJsonArray wordsSerialize(const QVector<WordEntry>& words) {
  QJsonArray array{};
  for (const auto& word : words) {
    QJsonObject obj;
    obj.insert("word", word.word);
    obj.insert("definition", word.definition);
    obj.insert("createdAt", word.createdAt.toString(Qt::ISODate));
    obj.insert("modifiedAt", word.modifiedAt.toString(Qt::ISODate));
    obj.insert("priority", word.priority);
    obj.insert("totalAttempts", word.totalAttempts);
    obj.insert("totalErrors", word.totalErrors);
    array.append(obj);
  }
  return array;
}

static QVector<PracticeEntry> practiceEntriesDeserialize(
    const QJsonArray& practiceEntryArray) {
  QVector<PracticeEntry> entries;
  for (const auto& value : practiceEntryArray) {
    if (!value.isObject()) continue;
    QJsonObject wordObj = value.toObject();
    QString word = wordObj.value("word").toString();
    QString definition = wordObj.value("definition").toString();
    QString userAnswer = wordObj.value("userAnswer").toString();
    if (word.isEmpty() || definition.isEmpty() || userAnswer.isEmpty())
      continue;
    entries.append({word, definition, userAnswer});
  }
  return entries;
}

static std::optional<PracticeRecord> practiceRecordDeserialize(
    const QJsonObject& obj) {
  PracticeRecord record;

  if (obj.value("practiceTime").isUndefined()) return std::nullopt;
  QDateTime practiceTime =
      QDateTime::fromString(obj["practiceTime"].toString(), Qt::ISODate);
  if (!practiceTime.isValid()) return std::nullopt;
  record.practiceTime = std::move(practiceTime);

  if (obj.value("duration").isUndefined()) return std::nullopt;
  QTime duration = QTime::fromString(obj["duration"].toString(), "hh:mm:ss");
  if (!duration.isValid()) return std::nullopt;
  record.duration = std::move(duration);

  if (!obj.value("correctWords").isArray()) return std::nullopt;
  QJsonArray correctWordsArray = obj.value("correctWords").toArray();
  record.correctWords = practiceEntriesDeserialize(correctWordsArray);

  if (!obj.value("wrongWords").isArray()) return std::nullopt;
  QJsonArray wrongWordsArray = obj.value("wrongWords").toArray();
  record.wrongWords = practiceEntriesDeserialize(wrongWordsArray);

  if (obj.value("correctRate").isUndefined()) return std::nullopt;
  record.correctRate = obj.value("correctRate").toDouble();

  if (obj.value("mode").isUndefined()) return std::nullopt;
  record.mode = static_cast<PracticeMode>(obj.value("mode").toInt());

  return record;
}

static QVector<PracticeRecord> practiceRecordsDeserialize(
    const QJsonArray& recordsArray) {
  QVector<PracticeRecord> records{};
  for (const auto& value : recordsArray) {
    if (!value.isObject()) continue;
    QJsonObject recordObj = value.toObject();
    auto record = practiceRecordDeserialize(recordObj);
    if (record.has_value()) {
      records.append(record.value());
    }
  }
  return records;
}

static QVector<WordEntry> wordsDeserialize(const QJsonArray& wordsArray) {
  QVector<WordEntry> words{};
  QDateTime now = QDateTime::currentDateTime();
  for (const auto& value : wordsArray) {
    if (!value.isObject()) continue;

    const QJsonObject obj = value.toObject();

    if (!obj.contains("word")) continue;
    QString word = obj.value("word").toString();

    if (!obj.contains("definition")) continue;
    QString definition = obj.value("definition").toString();

    QDateTime createdAt =
        QDateTime::fromString(obj.value("createdAt").toString(), Qt::ISODate);
    QDateTime modifiedAt =
        QDateTime::fromString(obj.value("modifiedAt").toString(), Qt::ISODate);
    if (!createdAt.isValid()) {
      createdAt = now;
      modifiedAt = now;
    }
    if (!modifiedAt.isValid()) {
      modifiedAt = now;
    }

    int priority = obj.value("priority").toInt(1);
    if (priority <= 0) priority = 1;

    int totalAttempts = obj.value("totalAttempts").toInt(0);
    int totalErrors = obj.value("totalErrors").toInt(0);
    if (totalErrors > totalAttempts) {
      totalErrors = totalAttempts;
    }

    words.append({word, definition, createdAt, modifiedAt, priority,
                  totalAttempts, totalErrors});
  }
  return words;
}

static std::optional<WordDatabaseError> readJsonArrayFile(
    const QString& filename, QJsonArray& array) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    return "无法打开文件：" + filename;
  }
  QByteArray data = file.readAll();
  file.close();
  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isArray()) {
    return "文件中的 JSON 格式无效：" + filename;
  }
  array = doc.array();
  return std::nullopt;
}

static std::optional<WordDatabaseError> writeJsonArrayFile(
    const QString& filename, const QJsonArray& array) {
  QJsonDocument doc = QJsonDocument(array);
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
    return "无法打开文件：" + filename;
  }
  file.write(doc.toJson());
  file.close();
  return std::nullopt;
}

std::optional<WordDatabaseError> WordDatabase::loadWordsFromFile() {
  QJsonArray jsonArray;
  auto err = readJsonArrayFile(m_filename, jsonArray);
  if (err) {
    return err;
  }
  m_words = wordsDeserialize(jsonArray);
  return std::nullopt;
}

std::optional<WordDatabaseError> WordDatabase::loadPracticeRecordsFromFile() {
  QJsonArray jsonArray;
  auto err = readJsonArrayFile(m_practiceRecordsFilename, jsonArray);
  if (err) {
    return err;
  }
  m_practiceRecords = practiceRecordsDeserialize(jsonArray);
  return std::nullopt;
}

std::optional<WordDatabaseError> WordDatabase::saveWordsToFile() const {
  QJsonArray jsonArray = wordsSerialize(m_words);
  return writeJsonArrayFile(m_filename, jsonArray);
}

std::optional<WordDatabaseError> WordDatabase::savePracticeRecordsToFile()
    const {
  QJsonArray jsonArray = practiceRecordsSerialize(m_practiceRecords);
  return writeJsonArrayFile(m_practiceRecordsFilename, jsonArray);
}

std::optional<WordDatabaseError> WordDatabase::updateWord(
    const QString& oldWord, const QString& newWord,
    const QString& newDefinition, int newPriority) {
  if (newWord.isEmpty()) {
    return "新单词不能为空";
  }

  if (newDefinition.isEmpty()) {
    return "新定义不能为空";
  }

  for (qsizetype i = 0; i < m_words.size(); ++i) {
    if (m_words[i].word == oldWord) {
      QDateTime now = QDateTime::currentDateTime();
      if (newWord != oldWord) {
        m_words[i].word = newWord;
        m_words[i].createdAt = now;
      }
      m_words[i].definition = newDefinition;
      m_words[i].priority = newPriority;
      m_words[i].modifiedAt = now;
      return std::nullopt;
    }
  }

  return "单词不存在";
}
