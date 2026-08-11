#ifndef PRACTICE_RESULT_DELEGATE_H
#define PRACTICE_RESULT_DELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief 练习结果列表的自定义委托
 *
 * 正确项显示绿色背景，错误项显示红色背景。
 * 根据 QListWidgetItem 的 Qt::UserRole 布尔值判断正误。
 */
class PracticeResultDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit PracticeResultDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif  // PRACTICE_RESULT_DELEGATE_H
