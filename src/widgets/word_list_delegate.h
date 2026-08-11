#ifndef WORD_LIST_DELEGATE_H
#define WORD_LIST_DELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief 单词列表自定义委托
 *
 * 每个列表项展示三部分信息：
 * - 左侧：单词文本（粗体）+ 释义预览（灰色，自动截断）
 * - 右侧：优先级徽章（根据优先级区间着色）
 *
 * 优先级配色：
 *   1-5  蓝色 #409eff（普通）
 *   6-10 橙色 #e6a23c（中等）
 *   11-15 红色 #f56c6c（重要）
 *   16-20 紫色 #9b59b6（核心）
 */
class WordListDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit WordListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif  // WORD_LIST_DELEGATE_H
