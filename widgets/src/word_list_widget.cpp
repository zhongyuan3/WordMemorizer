#include "widgets/word_list_widget.h"

#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QModelIndex>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

#include "models/word_list_model.h"

WordListWidget::WordListWidget(QWidget *parent)
    : QWidget(parent), m_proxyModel(nullptr) {
    // ---- 搜索框 ----
    m_searchEdit = new QLineEdit;
    m_searchEdit->setObjectName("wordSearchEdit");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setPlaceholderText("搜索单词...");

    m_searchCancelButton = new QPushButton;
    m_searchCancelButton->setObjectName("wordSearchCancelButton");
    m_searchCancelButton->setText("取消");
    m_searchCancelButton->hide();

    // ---- 排序组合框 ----
    m_sortCombo = new QComboBox;
    m_sortCombo->setObjectName("wordSortCombo");
    m_sortCombo->addItem("按创建时间升序");
    m_sortCombo->addItem("按创建时间降序");
    m_sortCombo->addItem("按修改时间升序");
    m_sortCombo->addItem("按修改时间降序");
    m_sortCombo->addItem("按优先级升序");
    m_sortCombo->addItem("按优先级降序");
    m_sortCombo->addItem("按字母升序");
    m_sortCombo->addItem("按字母降序");

    // ---- 列表视图 ----
    m_listView = new QListView;
    m_listView->setObjectName("wordListView");
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setMouseTracking(true);

    // ---- 单词计数标签 ----
    m_countLabel = new QLabel;
    m_countLabel->setObjectName("wordCountLabel");

    // ---- 右键菜单 ----
    m_contextMenu = new QMenu(this);
    m_viewDefinitionAction = m_contextMenu->addAction("查看释义");
    m_contextMenu->addSeparator();
    m_editAction = m_contextMenu->addAction("编辑...");
    m_deleteAction = m_contextMenu->addAction("删除");

    // ---- 布局 ----
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->setSpacing(8);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchCancelButton);

    QHBoxLayout *toolbarLayout = new QHBoxLayout;
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(8);
    toolbarLayout->addWidget(m_countLabel);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_sortCombo);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);
    mainLayout->addLayout(searchLayout);
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_listView);

    // ---- 信号连接 ----
    connect(m_searchEdit, &QLineEdit::textChanged, this,
            &WordListWidget::onSearchChanged);
    connect(m_searchCancelButton, &QPushButton::clicked, m_searchEdit,
            &QLineEdit::clear);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WordListWidget::onSortChanged);
    connect(m_listView, &QListView::customContextMenuRequested, this,
            &WordListWidget::onContextMenu);

    // 编辑/删除：通过 proxy->mapToSource 把当前 proxy index 换算成源行号
    connect(m_editAction, &QAction::triggered, this, [this]() {
        if (!m_proxyModel) return;
        QModelIndex proxyIndex = m_listView->currentIndex();
        if (!proxyIndex.isValid()) return;
        QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        if (!sourceIndex.isValid()) return;
        emit editRequested(sourceIndex.row());
    });

    connect(m_deleteAction, &QAction::triggered, this, [this]() {
        if (!m_proxyModel) return;
        QModelIndex proxyIndex = m_listView->currentIndex();
        if (!proxyIndex.isValid()) return;
        QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        if (!sourceIndex.isValid()) return;
        emit deleteRequested(sourceIndex.row());
    });

    connect(m_viewDefinitionAction, &QAction::triggered, this, [this]() {
        if (!m_proxyModel) return;
        QModelIndex proxyIndex = m_listView->currentIndex();
        if (!proxyIndex.isValid()) return;
        QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        if (!sourceIndex.isValid()) return;
        emit viewDefinitionRequested(sourceIndex.row());
    });
}

void WordListWidget::setModel(QAbstractItemModel *sourceModel,
                              QSortFilterProxyModel *proxyModel) {
    Q_UNUSED(sourceModel);

    m_proxyModel = proxyModel;
    m_listView->setModel(proxyModel);

    // 模型重置时更新计数标签
    connect(proxyModel, &QAbstractItemModel::modelReset, this,
            &WordListWidget::updateCountLabel);

    // 选中项变化时，把 proxy 选中映射成源行号抛出
    if (QItemSelectionModel *selModel = m_listView->selectionModel()) {
        connect(selModel, &QItemSelectionModel::currentChanged, this,
                &WordListWidget::onCurrentChanged);
    }

    // 用当前搜索/排序状态初始化代理模型
    onSearchChanged();
    onSortChanged(m_sortCombo->currentIndex());
    updateCountLabel();
}

void WordListWidget::refreshSelection() {
    onCurrentChanged(m_listView->currentIndex());
}

int WordListWidget::currentSourceRow() const {
    if (!m_proxyModel) return -1;
    QModelIndex proxyIndex = m_listView->currentIndex();
    if (!proxyIndex.isValid()) return -1;
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceIndex.row() : -1;
}

void WordListWidget::onSearchChanged() {
    const QString text = m_searchEdit->text();

    // 有输入内容时才显示取消按钮
    m_searchCancelButton->setVisible(!text.isEmpty());

    if (!m_proxyModel) return;

    // 大小写不敏感；filterFixedString 做的是“包含”匹配，
    // 即 Qt::MatchContains 语义。
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterFixedString(text);
    updateCountLabel();
}

void WordListWidget::onSortChanged(int index) {
    if (!m_proxyModel) return;

    const SortOrder order = static_cast<SortOrder>(index);

    int role = 0;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;

    switch (order) {
        case SortOrder::CreationTimeAsc:
            role = WordListModel::CreatedAtRole;
            sortOrder = Qt::AscendingOrder;
            break;
        case SortOrder::CreationTimeDesc:
            role = WordListModel::CreatedAtRole;
            sortOrder = Qt::DescendingOrder;
            break;
        case SortOrder::ModifiedAtAsc:
            role = WordListModel::ModifiedAtRole;
            sortOrder = Qt::AscendingOrder;
            break;
        case SortOrder::ModifiedAtDesc:
            role = WordListModel::ModifiedAtRole;
            sortOrder = Qt::DescendingOrder;
            break;
        case SortOrder::PriorityAsc:
            role = WordListModel::PriorityRole;
            sortOrder = Qt::AscendingOrder;
            break;
        case SortOrder::PriorityDesc:
            role = WordListModel::PriorityRole;
            sortOrder = Qt::DescendingOrder;
            break;
        case SortOrder::AlphabeticalAsc:
            role = WordListModel::WordTextRole;
            sortOrder = Qt::AscendingOrder;
            break;
        case SortOrder::AlphabeticalDesc:
            role = WordListModel::WordTextRole;
            sortOrder = Qt::DescendingOrder;
            break;
    }

    m_proxyModel->setSortRole(role);
    m_proxyModel->sort(0, sortOrder);
}

void WordListWidget::onContextMenu(const QPoint &pos) {
    const QModelIndex index = m_listView->indexAt(pos);
    if (!index.isValid()) return;

    // 把右键所在项设为当前项，便于 edit/delete 取 currentIndex
    m_listView->setCurrentIndex(index);

    m_contextMenu->exec(m_listView->viewport()->mapToGlobal(pos));
}

void WordListWidget::onCurrentChanged(const QModelIndex &current) {
    if (!m_proxyModel || !current.isValid()) return;

    const QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
    if (!sourceIndex.isValid()) return;

    emit wordSelected(sourceIndex.row());
}

void WordListWidget::updateCountLabel() {
    if (!m_proxyModel) return;
    auto *src = m_proxyModel->sourceModel();
    int total = src ? src->rowCount() : 0;
    int filtered = m_proxyModel->rowCount();
    if (filtered == total) {
        m_countLabel->setText(QString("共 %1 个单词").arg(total));
    } else {
        m_countLabel->setText(
            QString("找到 %1 / %2 个单词").arg(filtered).arg(total));
    }
}
