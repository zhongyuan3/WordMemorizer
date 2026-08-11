#ifndef PRACTICE_RECORD_MODEL_H
#define PRACTICE_RECORD_MODEL_H

#include <QAbstractListModel>

#include "data/models.h"
#include "data/word_database.h"

/**
 * @brief 练习记录列表的 QAbstractListModel
 */
class PracticeRecordModel : public QAbstractListModel {
    Q_OBJECT

public:
    static constexpr int RecordIndexRole = Qt::UserRole + 1;

    explicit PracticeRecordModel(WordDatabase *db, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    void refreshAll();

private:
    WordDatabase *m_db;
};

#endif  // PRACTICE_RECORD_MODEL_H
