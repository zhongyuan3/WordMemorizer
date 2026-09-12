#include "models/practice_record_model.h"

PracticeRecordModel::PracticeRecordModel(WordDatabase *db, QObject *parent)
    : QAbstractListModel(parent), m_db(db) {}

int PracticeRecordModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_db->practiceRecordCount();
}

QVariant PracticeRecordModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    int row = index.row();
    const auto &records = m_db->practiceRecords();
    if (row < 0 || row >= records.size()) return QVariant();

    const PracticeRecord &record = records[row];

    switch (role) {
        case Qt::DisplayRole:
            return QString("%1 | 时长 %2 | 总数 %3 | 正确率 %4%")
                .arg(record.practiceTime.toString("yyyy-MM-dd hh:mm:ss"))
                .arg(record.duration.toString("hh:mm:ss"))
                .arg(record.wordCount())
                .arg(record.correctRate, 0, 'f', 2);
        case RecordIndexRole:
            return row;
        default:
            return QVariant();
    }
}

void PracticeRecordModel::refreshAll() {
    beginResetModel();
    endResetModel();
}
