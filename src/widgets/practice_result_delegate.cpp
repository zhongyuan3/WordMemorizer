#include "widgets/practice_result_delegate.h"

#include <QBrush>
#include <QPainter>
#include <QPalette>

PracticeResultDelegate::PracticeResultDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void PracticeResultDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const {
    bool isError = index.data(Qt::UserRole).toBool();

    QStyleOptionViewItem opt = option;
    if (isError) {
        opt.backgroundBrush = QBrush(QColor(255, 220, 220));
        opt.palette.setBrush(QPalette::Text, QColor(200, 0, 0));
    } else {
        opt.backgroundBrush = QBrush(QColor(220, 255, 220));
        opt.palette.setBrush(QPalette::Text, QColor(0, 150, 0));
    }

    QStyledItemDelegate::paint(painter, opt, index);
}

QSize PracticeResultDelegate::sizeHint(const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const {
    return QStyledItemDelegate::sizeHint(option, index);
}
