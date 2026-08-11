#include "widgets/word_list_delegate.h"

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QModelIndex>
#include <QPainter>
#include <QPalette>
#include <QStyleOptionViewItem>

#include "models/word_list_model.h"

static constexpr int ITEM_HEIGHT = 56;
static constexpr int BADGE_SIZE = 26;
static constexpr int H_PADDING = 14;
static constexpr int V_PADDING = 8;

static QColor priorityColor(int priority) {
    if (priority <= 5) return QColor("#409eff");
    if (priority <= 10) return QColor("#e6a23c");
    if (priority <= 15) return QColor("#f56c6c");
    return QColor("#9b59b6");
}

WordListDelegate::WordListDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void WordListDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
    painter->save();

    // ---- 背景 ----
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor("#ecf5ff"));
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, QColor("#f5f7fa"));
    }

    // ---- 底部分割线 ----
    painter->setPen(QColor("#ebeef5"));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

    // ---- 数据 ----
    const QString word =
        index.data(WordListModel::WordTextRole).toString();
    const QString preview =
        index.data(WordListModel::DefinitionPreviewRole).toString();
    const int priority = index.data(WordListModel::PriorityRole).toInt();

    // ---- 内容区域 ----
    QRect contentRect = option.rect.adjusted(H_PADDING, V_PADDING,
                                             -H_PADDING, -V_PADDING);

    // ---- 右侧优先级徽章 ----
    QColor badgeColor = priorityColor(priority);
    QRect badgeRect(contentRect.right() - BADGE_SIZE,
                    contentRect.center().y() - BADGE_SIZE / 2, BADGE_SIZE,
                    BADGE_SIZE);
    painter->setBrush(badgeColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(badgeRect, 5, 5);

    painter->setPen(Qt::white);
    QFont badgeFont = option.font;
    badgeFont.setBold(true);
    badgeFont.setPointSize(9);
    painter->setFont(badgeFont);
    painter->drawText(badgeRect, Qt::AlignCenter, QString::number(priority));

    // ---- 左侧单词文本 ----
    int textRight = badgeRect.left() - 10;
    QRect textRect(contentRect.left(), contentRect.top(),
                   textRight - contentRect.left(), contentRect.height());

    QFont wordFont = option.font;
    wordFont.setBold(true);
    wordFont.setPointSize(11);
    painter->setFont(wordFont);
    painter->setPen(QColor("#303133"));

    QFontMetrics wordFm(wordFont);
    int wordHeight = wordFm.height();
    QRect wordRect(textRect.left(), textRect.top(), textRect.width(),
                   wordHeight);
    QString elidedWord =
        wordFm.elidedText(word, Qt::ElideRight, wordRect.width());
    painter->drawText(wordRect, Qt::AlignLeft | Qt::AlignVCenter, elidedWord);

    // ---- 释义预览 ----
    QFont defFont = option.font;
    defFont.setBold(false);
    defFont.setPointSize(9);
    painter->setFont(defFont);
    painter->setPen(QColor("#909399"));

    QFontMetrics defFm(defFont);
    QRect defRect(textRect.left(), textRect.top() + wordHeight + 2,
                  textRect.width(), defFm.height());
    QString elidedDef =
        defFm.elidedText(preview, Qt::ElideRight, defRect.width());
    painter->drawText(defRect, Qt::AlignLeft | Qt::AlignVCenter, elidedDef);

    painter->restore();
}

QSize WordListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {
    Q_UNUSED(index);
    return QSize(option.rect.width(), ITEM_HEIGHT);
}
