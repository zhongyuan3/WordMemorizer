#ifndef WORD_LIST_MODEL_H
#define WORD_LIST_MODEL_H

#include <QAbstractListModel>

#include "data/models.h"
#include "data/word_database.h"

/**
 * @brief 单词列表的 QAbstractListModel
 *
 * 重构要点：用标准 Model/View 架构替代旧版手动操作 QListWidget。
 * 排序和搜索由 QSortFilterProxyModel 在外层处理，模型本身只提供数据。
 */
class WordListModel : public QAbstractListModel {
    Q_OBJECT

public:
    /// 自定义数据角色
    static constexpr int WordIndexRole = Qt::UserRole + 1;       // 数据库中的索引
    static constexpr int WordTextRole = Qt::UserRole + 2;        // 单词文本
    static constexpr int PriorityRole = Qt::UserRole + 3;        // 优先级
    static constexpr int CreatedAtRole = Qt::UserRole + 4;       // 创建时间
    static constexpr int ModifiedAtRole = Qt::UserRole + 5;      // 修改时间
    static constexpr int DefinitionPreviewRole = Qt::UserRole + 6;  // 释义预览（第一条）

    explicit WordListModel(WordDatabase *db, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    /// 数据库发生变化后调用，整体刷新模型
    void refreshAll();

private:
    WordDatabase *m_db;
};

#endif  // WORD_LIST_MODEL_H
