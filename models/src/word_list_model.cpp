#include "models/word_list_model.h"

WordListModel::WordListModel(WordDatabase *db, QObject *parent)
    : QAbstractListModel(parent), m_db(db) {}

int WordListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_db->wordCount();
}

QVariant WordListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    int row = index.row();
    const auto &words = m_db->words();
    if (row < 0 || row >= words.size()) return QVariant();

    const WordEntry &entry = words[row];

    switch (role) {
        case Qt::DisplayRole:
            return entry.word;
        case Qt::ToolTipRole: {
            QString tip = QString("单词: %1\n优先级: %2").arg(entry.word).arg(entry.priority);
            if (!entry.definitions.isEmpty()) {
                tip += "\n释义:";
                for (const QString &d : entry.definitions) {
                    tip += QString("\n  %1").arg(d);
                }
            }
            return tip;
        }
        case WordIndexRole:
            return row;
        case WordTextRole:
            return entry.word;
        case PriorityRole:
            return entry.priority;
        case CreatedAtRole:
            return entry.createdAt;
        case ModifiedAtRole:
            return entry.modifiedAt;
        case DefinitionPreviewRole:
            return entry.definitions.isEmpty() ? QString("（无释义）")
                                               : entry.definitions.first();
        default:
            return QVariant();
    }
}

void WordListModel::refreshAll() {
    beginResetModel();
    endResetModel();
}
