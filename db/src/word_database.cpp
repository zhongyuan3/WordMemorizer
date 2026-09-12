#include "data/word_database.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// ============================================================
// 查询接口
// ============================================================

bool WordDatabase::containsWord(const QString &word) const {
    return m_wordIndex.contains(word);
}

const WordEntry *WordDatabase::findWord(const QString &word) const {
    auto it = m_wordIndex.constFind(word);
    if (it == m_wordIndex.constEnd()) return nullptr;
    return &m_words[*it];
}

// ============================================================
// 修改接口
// ============================================================

std::optional<DatabaseError> WordDatabase::addWord(
    const QString &word, const QStringList &definitions, int priority) {
    if (word.isEmpty()) {
        return DatabaseError("单词不能为空");
    }
    if (definitions.isEmpty()) {
        return DatabaseError("释义不能为空");
    }
    if (containsWord(word)) {
        return DatabaseError("单词已存在");
    }

    QDateTime now = QDateTime::currentDateTime();
    WordEntry entry;
    entry.word = word.trimmed();
    entry.definitions = definitions;
    entry.createdAt = now;
    entry.modifiedAt = now;
    entry.priority = priority;

    m_wordIndex.insert(entry.word, m_words.size());
    m_words.append(std::move(entry));
    return std::nullopt;
}

std::optional<DatabaseError> WordDatabase::removeWord(const QString &word) {
    auto it = m_wordIndex.constFind(word);
    if (it == m_wordIndex.constEnd()) {
        return DatabaseError("单词不存在");
    }

    qsizetype idx = *it;
    m_words.removeAt(idx);
    // 重建索引（removeAt 后所有 idx 之后的元素前移）
    rebuildIndex();
    return std::nullopt;
}

std::optional<DatabaseError> WordDatabase::updateWord(
    const QString &oldWord, const QString &newWord,
    const QStringList &newDefinitions, int newPriority) {
    if (newWord.isEmpty()) {
        return DatabaseError("新单词不能为空");
    }
    if (newDefinitions.isEmpty()) {
        return DatabaseError("新释义不能为空");
    }

    auto it = m_wordIndex.constFind(oldWord);
    if (it == m_wordIndex.constEnd()) {
        return DatabaseError("单词不存在");
    }

    qsizetype idx = *it;

    // 如果改了单词文本，需要检查是否与已有单词冲突
    if (newWord != oldWord && containsWord(newWord)) {
        return DatabaseError("新单词与其他已有单词重复");
    }

    WordEntry &entry = m_words[idx];
    QDateTime now = QDateTime::currentDateTime();

    if (newWord != oldWord) {
        m_wordIndex.remove(oldWord);
        entry.word = newWord.trimmed();
        entry.createdAt = now;  // 改名视为新词，重置创建时间
        m_wordIndex.insert(entry.word, idx);
    }
    entry.definitions = newDefinitions;
    entry.priority = newPriority;
    entry.modifiedAt = now;
    return std::nullopt;
}

void WordDatabase::recordAttempt(const QString &word, bool wasCorrect) {
    auto it = m_wordIndex.constFind(word);
    if (it == m_wordIndex.constEnd()) return;

    WordEntry &entry = m_words[*it];
    entry.totalAttempts++;
    if (!wasCorrect) {
        entry.totalErrors++;
    }
    entry.modifiedAt = QDateTime::currentDateTime();
}

void WordDatabase::addPracticeRecord(const PracticeRecord &record) {
    m_practiceRecords.append(record);
}

// ============================================================
// 索引维护
// ============================================================

void WordDatabase::rebuildIndex() {
    m_wordIndex.clear();
    m_wordIndex.reserve(m_words.size());
    for (qsizetype i = 0; i < m_words.size(); ++i) {
        m_wordIndex.insert(m_words[i].word, i);
    }
}

// ============================================================
// 序列化辅助函数
// ============================================================

static QJsonArray practiceEntriesSerialize(
    const QVector<PracticeEntry> &entries) {
    QJsonArray array;
    for (const auto &e : entries) {
        QJsonObject obj;
        obj.insert("word", e.word);
        obj.insert("definition", e.definition);
        obj.insert("userAnswer", e.userAnswer);
        array.append(obj);
    }
    return array;
}

static QJsonObject practiceRecordSerialize(const PracticeRecord &r) {
    QJsonObject obj;
    obj.insert("practiceTime", r.practiceTime.toString(Qt::ISODate));
    obj.insert("duration", r.duration.toString("hh:mm:ss"));
    obj.insert("correctWords", practiceEntriesSerialize(r.correctWords));
    obj.insert("wrongWords", practiceEntriesSerialize(r.wrongWords));
    obj.insert("correctRate", r.correctRate);
    obj.insert("mode", static_cast<int>(r.mode));
    return obj;
}

static QJsonArray practiceRecordsSerialize(
    const QVector<PracticeRecord> &records) {
    QJsonArray array;
    for (const auto &r : records) {
        array.append(practiceRecordSerialize(r));
    }
    return array;
}

static QJsonArray wordsSerialize(const QVector<WordEntry> &words) {
    QJsonArray array;
    for (const auto &w : words) {
        QJsonObject obj;
        obj.insert("word", w.word);
        // 新格式：definitions 为 JSON 数组
        QJsonArray defsArray;
        for (const auto &d : w.definitions) {
            defsArray.append(d);
        }
        obj.insert("definitions", defsArray);
        // 兼容旧格式：同时写入 definition 字段
        obj.insert("definition", w.definitionsText());
        obj.insert("createdAt", w.createdAt.toString(Qt::ISODate));
        obj.insert("modifiedAt", w.modifiedAt.toString(Qt::ISODate));
        obj.insert("priority", w.priority);
        obj.insert("totalAttempts", w.totalAttempts);
        obj.insert("totalErrors", w.totalErrors);
        array.append(obj);
    }
    return array;
}

// ---- 反序列化 ----

static QVector<PracticeEntry> practiceEntriesDeserialize(
    const QJsonArray &array) {
    QVector<PracticeEntry> entries;
    for (const auto &value : array) {
        if (!value.isObject()) continue;
        QJsonObject obj = value.toObject();
        QString word = obj.value("word").toString();
        QString definition = obj.value("definition").toString();
        QString userAnswer = obj.value("userAnswer").toString();
        if (word.isEmpty() || definition.isEmpty() || userAnswer.isEmpty())
            continue;
        entries.append({word, definition, userAnswer});
    }
    return entries;
}

static std::optional<PracticeRecord> practiceRecordDeserialize(
    const QJsonObject &obj) {
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
    record.correctWords =
        practiceEntriesDeserialize(obj.value("correctWords").toArray());

    if (!obj.value("wrongWords").isArray()) return std::nullopt;
    record.wrongWords =
        practiceEntriesDeserialize(obj.value("wrongWords").toArray());

    if (obj.value("correctRate").isUndefined()) return std::nullopt;
    record.correctRate = obj.value("correctRate").toDouble();

    if (obj.value("mode").isUndefined()) return std::nullopt;
    record.mode = static_cast<PracticeMode>(obj.value("mode").toInt());

    return record;
}

static QVector<PracticeRecord> practiceRecordsDeserialize(
    const QJsonArray &array) {
    QVector<PracticeRecord> records;
    for (const auto &value : array) {
        if (!value.isObject()) continue;
        auto record = practiceRecordDeserialize(value.toObject());
        if (record.has_value()) {
            records.append(record.value());
        }
    }
    return records;
}

static QVector<WordEntry> wordsDeserialize(const QJsonArray &array) {
    QVector<WordEntry> words;
    QDateTime now = QDateTime::currentDateTime();

    for (const auto &value : array) {
        if (!value.isObject()) continue;
        QJsonObject obj = value.toObject();

        if (!obj.contains("word")) continue;
        QString word = obj.value("word").toString();

        QStringList definitions;
        // 优先读取新格式 definitions 数组
        if (obj.value("definitions").isArray()) {
            QJsonArray defsArray = obj.value("definitions").toArray();
            for (const auto &d : defsArray) {
                definitions.append(d.toString());
            }
        }
        // 兼容旧格式 definition 字符串
        if (definitions.isEmpty() && obj.contains("definition")) {
            definitions =
                WordEntry::parseDefinitions(obj.value("definition").toString());
        }
        if (definitions.isEmpty()) continue;

        QDateTime createdAt = QDateTime::fromString(
            obj.value("createdAt").toString(), Qt::ISODate);
        QDateTime modifiedAt = QDateTime::fromString(
            obj.value("modifiedAt").toString(), Qt::ISODate);
        if (!createdAt.isValid()) createdAt = now;
        if (!modifiedAt.isValid()) modifiedAt = now;

        int priority = obj.value("priority").toInt(1);
        if (priority <= 0) priority = 1;

        int totalAttempts = obj.value("totalAttempts").toInt(0);
        int totalErrors = obj.value("totalErrors").toInt(0);
        if (totalErrors > totalAttempts) totalErrors = totalAttempts;

        WordEntry entry;
        entry.word = word;
        entry.definitions = definitions;
        entry.createdAt = createdAt;
        entry.modifiedAt = modifiedAt;
        entry.priority = priority;
        entry.totalAttempts = totalAttempts;
        entry.totalErrors = totalErrors;
        words.append(std::move(entry));
    }
    return words;
}

// ---- 文件读写辅助 ----

static std::optional<DatabaseError> readJsonArrayFile(const QString &filename,
                                                      QJsonArray &array) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return DatabaseError("无法打开文件：" + filename);
    }
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        return DatabaseError("文件中的 JSON 格式无效：" + filename);
    }
    array = doc.array();
    return std::nullopt;
}

static std::optional<DatabaseError> writeJsonArrayFile(
    const QString &filename, const QJsonArray &array) {
    QJsonDocument doc(array);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return DatabaseError("无法打开文件：" + filename);
    }
    file.write(doc.toJson());
    file.close();
    return std::nullopt;
}

// ============================================================
// 持久化
// ============================================================

std::optional<DatabaseError> WordDatabase::saveToFile() const {
    auto err = saveWordsToFile();
    if (err) return err;
    err = savePracticeRecordsToFile();
    return err;
}

std::optional<DatabaseError> WordDatabase::loadFromFile() {
    auto err = loadWordsFromFile();
    if (err) return err;
    err = loadPracticeRecordsFromFile();
    return err;
}

std::optional<DatabaseError> WordDatabase::loadWordsFromFile() {
    QJsonArray array;
    auto err = readJsonArrayFile(m_wordsFilename, array);
    if (err) return err;
    m_words = wordsDeserialize(array);
    rebuildIndex();
    return std::nullopt;
}

std::optional<DatabaseError> WordDatabase::loadPracticeRecordsFromFile() {
    QJsonArray array;
    auto err = readJsonArrayFile(m_practiceRecordsFilename, array);
    if (err) return err;
    m_practiceRecords = practiceRecordsDeserialize(array);
    return std::nullopt;
}

std::optional<DatabaseError> WordDatabase::saveWordsToFile() const {
    return writeJsonArrayFile(m_wordsFilename, wordsSerialize(m_words));
}

std::optional<DatabaseError> WordDatabase::savePracticeRecordsToFile() const {
    return writeJsonArrayFile(m_practiceRecordsFilename,
                              practiceRecordsSerialize(m_practiceRecords));
}
