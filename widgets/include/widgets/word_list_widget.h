#ifndef WORD_LIST_WIDGET_H
#define WORD_LIST_WIDGET_H

#include <QWidget>

class QListView;
class QLineEdit;
class QLabel;
class QPushButton;
class QComboBox;
class QMenu;
class QAction;
class QAbstractItemModel;
class QSortFilterProxyModel;

/**
 * @brief 单词列表 Widget
 *
 * 提供搜索、排序、右键菜单功能。基于 QListView + Model/View 架构，
 * 排序与过滤交由外部传入的 QSortFilterProxyModel 处理，本组件只负责
 * 组织界面、把用户操作映射成对代理模型的调用，并把以“源模型行号”为
 * 粒度的事件通过信号抛给上层。
 */
class WordListWidget : public QWidget {
    Q_OBJECT

public:
    /// 排序方式，顺序与排序组合框的选项一一对应
    enum class SortOrder {
        CreationTimeAsc,
        CreationTimeDesc,
        ModifiedAtAsc,
        ModifiedAtDesc,
        PriorityAsc,
        PriorityDesc,
        AlphabeticalAsc,
        AlphabeticalDesc
    };

    explicit WordListWidget(QWidget *parent = nullptr);

    /// 设置数据模型。sourceModel 提供数据，proxyModel 负责排序与过滤。
    /// proxyModel 的 source 应已设为 sourceModel。
    void setModel(QAbstractItemModel *sourceModel,
                  QSortFilterProxyModel *proxyModel);

    /// 获取当前选中项的源模型行号，无选中时返回 -1
    int currentSourceRow() const;

signals:
    /// 请求编辑指定行（源模型行号）
    void editRequested(int sourceRow);
    /// 请求删除指定行（源模型行号）
    void deleteRequested(int sourceRow);
    /// 选中某单词时发出（源模型行号）
    void wordSelected(int sourceRow);
    /// 请求查看单词释义（源模型行号）
    void viewDefinitionRequested(int sourceRow);

public slots:
    /// 刷新当前选中项（重新发出 wordSelected）
    void refreshSelection();

private slots:
    void onSearchChanged();
    void onSortChanged(int index);
    void onContextMenu(const QPoint &pos);
    void onCurrentChanged(const QModelIndex &current);
    void updateCountLabel();

private:
    QListView *m_listView;
    QLineEdit *m_searchEdit;
    QLabel *m_countLabel;
    QPushButton *m_searchCancelButton;
    QComboBox *m_sortCombo;
    QMenu *m_contextMenu;
    QAction *m_viewDefinitionAction;
    QAction *m_editAction;
    QAction *m_deleteAction;

    QSortFilterProxyModel *m_proxyModel;
};

#endif  // WORD_LIST_WIDGET_H
