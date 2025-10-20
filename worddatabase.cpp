#include "worddatabase.hpp"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

std::optional<WordDatabaseError> WordDatabase::addWord(const QString& word,
                                                       const QString& definition, int priority) {
    if (word.isEmpty()) {
        return std::optional<WordDatabaseError>("单词不能为空");
    }

    if (definition.isEmpty()) {
        return std::optional<WordDatabaseError>("定义不能为空");
    }

    if (containsWord(word)) {
        return std::optional<WordDatabaseError>("单词已存在");
    }

    QDateTime now = QDateTime::currentDateTime();

    WordEntry entry = {.word = word.trimmed(),
                       .definition = definition.trimmed(),
                       .createdAt = now,
                       .modifiedAt = now,
                       .priority = priority};

    m_words.append(entry);

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
    return std::optional<WordDatabaseError>("单词不存在");
}

std::optional<WordDatabaseError> WordDatabase::saveToFile() const {
    QJsonArray wordsArray;

    for (const auto& entry : m_words) {
        QJsonObject obj;
        obj["word"] = entry.word;
        obj["definition"] = entry.definition;
        obj["priority"] = entry.priority;
        obj["createdAt"] = entry.createdAt.toString(Qt::ISODate);
        obj["modifiedAt"] = entry.modifiedAt.toString(Qt::ISODate);
        wordsArray.append(obj);
    }

    QJsonDocument doc = QJsonDocument(wordsArray);

    QFile file = QFile(m_filename);

    if (!file.open(QIODevice::WriteOnly)) {
        return std::optional<WordDatabaseError>("无法打开文件：" + m_filename);
    }

    file.write(doc.toJson());
    file.close();
    return std::nullopt;
}

std::optional<WordDatabaseError> WordDatabase::loadFromFile() {
    QFile file(m_filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return std::optional<WordDatabaseError>("无法打开文件：" + m_filename);
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        return std::optional<WordDatabaseError>("文件中的 JSON 格式无效：" + m_filename);
    }

    const QJsonArray wordsArray = doc.array();
    m_words.clear();
    QDateTime now = QDateTime::currentDateTime();
    for (const auto& value : wordsArray) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            bool containsWord = obj.contains("word");
            bool containsDefinition = obj.contains("definition");
            bool containsPriority = obj.contains("priority");
            bool containsCreatedAt = obj.contains("createdAt");
            bool containsModifiedAt = obj.contains("modifiedAt");

            if (containsWord && containsDefinition && containsPriority && containsCreatedAt &&
                containsModifiedAt) {
                WordEntry entry = {
                    .word = obj["word"].toString(),
                    .definition = obj["definition"].toString(),
                    .createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate),
                    .modifiedAt = QDateTime::fromString(obj["modifiedAt"].toString(), Qt::ISODate),
                    .priority = obj["priority"].toInt()};
                m_words.append(entry);
            } else if (containsWord && containsDefinition && containsPriority) {
                WordEntry entry = {.word = obj["word"].toString(),
                                   .definition = obj["definition"].toString(),
                                   .createdAt = now,
                                   .modifiedAt = now,
                                   .priority = obj["priority"].toInt()};
                m_words.append(entry);
            } else if (containsWord && containsDefinition) {
                WordEntry entry = {.word = obj["word"].toString(),
                                   .definition = obj["definition"].toString(),
                                   .createdAt = now,
                                   .modifiedAt = now,
                                   .priority = 1};
                m_words.append(entry);
            }
        }
    }
    return std::nullopt;
}

std::optional<WordDatabaseError> WordDatabase::updateWord(const QString& oldWord,
                                                          const QString& newWord,
                                                          const QString& newDefinition,
                                                          int newPriority) {
    if (newWord.isEmpty()) {
        return std::optional<WordDatabaseError>("新单词不能为空");
    }

    if (newDefinition.isEmpty()) {
        return std::optional<WordDatabaseError>("新定义不能为空");
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

    return std::optional<WordDatabaseError>("单词不存在");
}
