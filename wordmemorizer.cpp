#include "wordmemorizer.hpp"

#include <QApplication>
#include <QDir>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

#include "algorithm.hpp"

// PracticeResultDelegate 类实现
PracticeResultDelegate::PracticeResultDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void PracticeResultDelegate::paint(QPainter* painter,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  // 检查是否是错误项（通过Qt::UserRole存储）
  bool isError = index.data(Qt::UserRole).toBool();

  QStyleOptionViewItem opt = option;

  if (isError) {
    // 错误项使用红色背景
    opt.backgroundBrush = QBrush(QColor(255, 220, 220));
    opt.palette.setBrush(QPalette::Text, QColor(200, 0, 0));
  } else {
    // 正确项使用绿色背景
    opt.backgroundBrush = QBrush(QColor(220, 255, 220));
    opt.palette.setBrush(QPalette::Text, QColor(0, 150, 0));
  }

  QStyledItemDelegate::paint(painter, opt, index);
}

QSize PracticeResultDelegate::sizeHint(const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
  // 使用基类的实现
  return QStyledItemDelegate::sizeHint(option, index);
}

static constexpr const char* STYLE_FILE_PATH = ":/styles/main.qss";
static constexpr const char* DATA_FILE_NAME = "words.json";
static constexpr const char* PRACTICE_RECORDS_FILE_NAME =
    "practice_records.json";

WordMemorizer::WordMemorizer(QWidget* parent)
    : QMainWindow(parent),
      m_currentIndex(-1),
      m_correctCount(0),
      m_totalQuestions(0),
      m_isSearching(false),
      m_currentMode(PracticeMode::WordToDefinition),
      m_wordListSortOrder(SortOrder::CreationTimeAsc),
      m_wordListDataCache(),
      m_elapsedSeconds(0),
      m_wordListContextMenu(nullptr),
      m_editWordAction(nullptr),
      m_deleteWordAction(nullptr),
      m_editDialog(nullptr) {
  QString writePath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (writePath.isEmpty()) {
    QMessageBox::critical(this, "错误", "获取写入目录失败");
    return;
  }

  QDir writeDir(writePath);
  if (!writeDir.exists() && !writeDir.mkpath(".")) {
    QMessageBox::critical(this, "错误", "创建写入目录失败");
    return;
  }

  m_db.setFilename(writeDir.absoluteFilePath(DATA_FILE_NAME));
  m_db.setPracticeRecordsFilename(
      writeDir.absoluteFilePath(PRACTICE_RECORDS_FILE_NAME));
  std::optional<WordDatabaseError> err = m_db.loadFromFile();
  if (err) {
    QMessageBox::critical(this, "错误", err.value().message());
  }

  setupUi();
  setupStyle();
  syncAndUpdateWordList();
  updatePracticeRecordConstViews();
  updatePracticeRecordList();
  loadSearchCompleterModel();
}

void WordMemorizer::onCurrentItemChanged(QListWidgetItem* curr,
                                         QListWidgetItem* prev) {
  if (prev) {
    qulonglong idx = prev->data(Qt::UserRole).toULongLong();
    const WordEntry& entry = m_db.getWords()[idx];
    prev->setText(entry.word);
  }

  if (curr) {
    qulonglong idx = curr->data(Qt::UserRole).toULongLong();
    const WordEntry& entry = m_db.getWords()[idx];
    curr->setText(entry.word + " - " + entry.definition);
  }
}

void WordMemorizer::onSortOrderChanged() {
  m_wordListSortOrder =
      static_cast<SortOrder>(m_wordListSortOrderCombo->currentIndex());
  updateWordList();
}

void WordMemorizer::onSearchEditReturnPressed() {
  QString searchText = m_searchEdit->text().trimmed();

  if (searchText.isEmpty() && m_isSearching) {
    m_isSearching = false;
    m_searchCancelButton->setHidden(true);
    syncAndUpdateWordList();
    return;
  }

  auto results = m_db.queryWords(searchText);
  if (results.isEmpty()) {
    m_wordListWidget->clear();
    QMessageBox::information(this, "提示", "未找到匹配单词");
    return;
  }

  if (!m_isSearching) {
    m_isSearching = true;
    m_searchCancelButton->setHidden(false);
  }

  m_wordListDataCache = results;
  updateWordList();
}

void WordMemorizer::onPracticeRecordDoubleClicked(QListWidgetItem* item) {
  if (!item) {
    return;
  }

  qulonglong idx = item->data(Qt::UserRole).toULongLong();
  const PracticeRecord& record = m_db.getPracticeRecords()[idx];
  m_practiceRecordDisplayer->displayRecord(record);
  m_practiceRecordDisplayer->show();
}

void WordMemorizer::setupUi() {
  QWidget* centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

  // 创建各个功能区域的UI组件
  QGroupBox* wordsGroup = createWordsGroup();
  QGroupBox* practiceRecordGroup = createPracticeRecordGroup();
  QGroupBox* practiceGroup = createPracticeGroup();

  // 创建编辑对话框
  createEditDialog();

  // 创建练习记录详情显示窗口
  m_practiceRecordDisplayer = new PracticeRecordDisplayer(this);
  m_practiceRecordDisplayer->setupUi();

  // 添加到主布局
  QHBoxLayout* layout = new QHBoxLayout();
  layout->addWidget(wordsGroup);
  layout->addWidget(practiceRecordGroup);
  mainLayout->addLayout(layout);
  mainLayout->addWidget(practiceGroup);

  // 设置信号槽连接
  setupConnections();
}

void WordMemorizer::setupConnections() {
  // 搜索相关连接
  connect(m_searchCancelButton, &QPushButton::clicked, this, [this]() {
    m_searchEdit->clear();
    onSearchEditReturnPressed();
  });
  connect(m_searchEdit, &QLineEdit::returnPressed, this,
          &WordMemorizer::onSearchEditReturnPressed);

  // 排序相关连接
  connect(m_wordListSortOrderCombo, &QComboBox::currentIndexChanged, this,
          &WordMemorizer::onSortOrderChanged);

  // 单词录入相关连接
  connect(m_addDefinitionButton, &QPushButton::clicked, this, [this]() {
    QString input = m_definitionEdit->text().trimmed();
    if (input.isEmpty()) {
      return;
    }
    const QStringList defs = input.split(" ");
    m_definitionListWidget->addItems(defs);
    m_definitionEdit->clear();
    m_definitionEdit->setFocus();
  });
  connect(m_removeDefinitionButton, &QPushButton::clicked, this, [this]() {
    QListWidgetItem* item = m_definitionListWidget->currentItem();
    if (item) {
      delete m_definitionListWidget->takeItem(
          m_definitionListWidget->row(item));
    }
  });

  // 单词操作相关连接
  connect(m_addButton, &QPushButton::clicked, this, &WordMemorizer::onAddWord);
  connect(m_deleteButton, &QPushButton::clicked, this,
          &WordMemorizer::onDeleteWord);

  // 练习相关连接
  connect(m_startPracticeButton, &QPushButton::clicked, this,
          &WordMemorizer::onStartPractice);
  connect(m_answerEdit, &QLineEdit::returnPressed, this,
          &WordMemorizer::onPracticeSubmit);
  connect(m_submitButton, &QPushButton::clicked, this,
          &WordMemorizer::onPracticeSubmit);
  connect(m_exitPracticeButton, &QPushButton::clicked, this,
          &WordMemorizer::onExitPractice);
  connect(m_practiceTimer, &QTimer::timeout, this,
          &WordMemorizer::onPracticeTimerTimeout);

  // 单词列表相关连接
  connect(m_wordListWidget, &QListWidget::customContextMenuRequested, this,
          &WordMemorizer::onWordListContextMenuRequested);
  connect(m_wordListWidget, &QListWidget::currentItemChanged, this,
          &WordMemorizer::onCurrentItemChanged);

  // 上下文菜单相关连接
  connect(m_editWordAction, &QAction::triggered, this,
          &WordMemorizer::onEditWord);
  connect(m_deleteWordAction, &QAction::triggered, this,
          &WordMemorizer::onDeleteWord);

  // 编辑对话框相关连接
  connect(m_editAddDefButton, &QPushButton::clicked, this, [this]() {
    QString input = m_editDefinitionInput->text().trimmed();
    if (input.isEmpty()) {
      return;
    }
    const QStringList defs = input.split(" ");
    m_editDefinitionList->addItems(defs);
    m_editDefinitionInput->clear();
    m_editDefinitionInput->setFocus();
  });
  connect(m_editRemoveDefButton, &QPushButton::clicked, this, [this]() {
    QListWidgetItem* item = m_editDefinitionList->currentItem();
    if (item) {
      delete m_editDefinitionList->takeItem(m_editDefinitionList->row(item));
    }
  });
  connect(m_editSaveButton, &QPushButton::clicked, this,
          &WordMemorizer::onSaveWordEdit);
  connect(m_editCancelButton, &QPushButton::clicked, this,
          &WordMemorizer::onCancelWordEdit);
  connect(m_editDefinitionInput, &QLineEdit::returnPressed, m_editAddDefButton,
          &QPushButton::click);

  // 应用退出时保存数据
  connect(qApp, &QApplication::aboutToQuit, this, [this]() { saveDatabase(); });

  // 练习记录详情显示窗口相关连接
  connect(m_practiceRecordListWidget, &QListWidget::itemDoubleClicked, this,
          &WordMemorizer::onPracticeRecordDoubleClicked);
}

QGroupBox* WordMemorizer::createWordsGroup() {
  // 创建单词录入区域
  QFormLayout* wordInputLayout = createWordInputLayout();

  // 创建单词列表区域
  QVBoxLayout* wordListLayout = createWordListLayout();

  // 创建并设置单词组
  QGroupBox* wordsGroup = new QGroupBox;
  wordsGroup->setObjectName("wordsGroup");
  QHBoxLayout* wordGroupLayout = new QHBoxLayout;
  wordGroupLayout->addLayout(wordInputLayout);
  wordGroupLayout->addLayout(wordListLayout);
  wordsGroup->setLayout(wordGroupLayout);

  return wordsGroup;
}

QFormLayout* WordMemorizer::createWordInputLayout() {
  m_wordEdit = new QLineEdit;
  m_wordEdit->setObjectName("m_wordEdit");
  m_definitionEdit = new QLineEdit;
  m_definitionEdit->setObjectName("m_definitionEdit");
  m_prioritySpin = new QSpinBox;
  m_prioritySpin->setObjectName("m_prioritySpin");
  m_prioritySpin->setRange(1, 20);
  m_prioritySpin->setValue(1);

  m_addDefinitionButton = new QPushButton("添加释义");
  m_addDefinitionButton->setObjectName("m_addDefinitionButton");
  QHBoxLayout* addDefLayout = new QHBoxLayout;
  addDefLayout->addWidget(m_definitionEdit);
  addDefLayout->addWidget(m_addDefinitionButton);

  m_definitionListWidget = new QListWidget;
  m_definitionListWidget->setObjectName("m_definitionListWidget");
  m_removeDefinitionButton = new QPushButton("删除选中的释义");
  m_removeDefinitionButton->setObjectName("m_removeDefinitionButton");

  m_addButton = new QPushButton("添加单词");
  m_addButton->setObjectName("m_addButton");
  m_deleteButton = new QPushButton("删除选中单词");
  m_deleteButton->setObjectName("m_deleteButton");
  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->addWidget(m_addButton);
  btnLayout->addWidget(m_deleteButton);

  QFormLayout* wordInputLayout = new QFormLayout;
  wordInputLayout->addRow("单词", m_wordEdit);
  wordInputLayout->addRow("优先级", m_prioritySpin);
  wordInputLayout->addRow("输入释义", addDefLayout);
  wordInputLayout->addRow("释义", m_definitionListWidget);
  wordInputLayout->addRow(m_removeDefinitionButton);
  wordInputLayout->addRow(btnLayout);

  return wordInputLayout;
}

QVBoxLayout* WordMemorizer::createWordListLayout() {
  // 创建单词列表
  m_wordListWidget = new QListWidget;
  m_wordListWidget->setObjectName("m_wordListWidget");

  // 创建搜索框
  m_searchCancelButton = new QPushButton("取消");
  m_searchCancelButton->setObjectName("m_searchCancelButton");
  m_searchCancelButton->setHidden(true);
  m_searchEdit = new QLineEdit;
  m_searchEdit->setObjectName("m_searchEdit");
  m_searchEdit->setClearButtonEnabled(true);
  QHBoxLayout* searchLayout = new QHBoxLayout;
  searchLayout->addWidget(m_searchEdit);
  searchLayout->addWidget(m_searchCancelButton);

  // 设置搜索自动完成
  setupSearchCompleter();

  // 创建排序组合框
  m_wordListSortOrderCombo = new QComboBox;
  m_wordListSortOrderCombo->setObjectName("m_wordListSortOrderCombo");
  m_wordListSortOrderCombo->addItem("按创建时间升序");
  m_wordListSortOrderCombo->addItem("按创建时间降序");
  m_wordListSortOrderCombo->addItem("按修改时间升序");
  m_wordListSortOrderCombo->addItem("按修改时间降序");
  m_wordListSortOrderCombo->addItem("按优先级升序");
  m_wordListSortOrderCombo->addItem("按优先级降序");
  m_wordListSortOrderCombo->addItem("按字母升序");
  m_wordListSortOrderCombo->addItem("按字母降序");

  // 创建单词列表布局
  QVBoxLayout* wordListLayout = new QVBoxLayout;
  wordListLayout->addLayout(searchLayout);
  wordListLayout->addWidget(m_wordListSortOrderCombo);
  wordListLayout->addWidget(m_wordListWidget);

  // 设置上下文菜单
  setupContextMenu();

  return wordListLayout;
}

void WordMemorizer::setupSearchCompleter() {
  m_searchCompleter = new QCompleter(this);
  m_searchCompleter->setObjectName("m_searchCompleter");
  m_searchCompleter->setCompletionMode(QCompleter::PopupCompletion);
  m_searchCompleter->setFilterMode(Qt::MatchContains);
  m_searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);

  m_searchModel = new QStringListModel(this);
  m_searchCompleter->setModel(m_searchModel);
  m_searchEdit->setCompleter(m_searchCompleter);

  // 设置搜索自动完成弹出窗口样式
  if (QListView* popup = qobject_cast<QListView*>(m_searchCompleter->popup())) {
    popup->setStyleSheet(R"(
        QListView {
            border: 2px solid #dcdfe6;
            border-radius: 6px;
            background-color: white;
            color: #2c3e50;
            font-size: 14px;
            outline: none;
            padding: 0px;
            margin: 0px;
            max-height: 200px;
        }
        QListView::item {
            padding: 8px 12px;
            border-bottom: 1px solid #ebeef5;
        }
        QListView::item:last {
            border-bottom: none;
        }
        QListView::item:selected {
            background-color: #ecf5ff;
            color: #409eff;
            border: 1px solid #409eff;
            border-radius: 4px;
        }
        QListView::item:hover {
            background-color: #f5f7fa;
        }
    )");
  }
}

void WordMemorizer::setupContextMenu() {
  m_editWordAction = new QAction("编辑单词", this);
  m_deleteWordAction = new QAction("删除单词", this);

  m_wordListContextMenu = new QMenu(this);
  m_wordListContextMenu->setObjectName("m_wordListContextMenu");
  m_wordListContextMenu->addAction(m_editWordAction);
  m_wordListContextMenu->addAction(m_deleteWordAction);

  m_wordListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

QGroupBox* WordMemorizer::createPracticeRecordGroup() {
  // 创建练习记录列表
  m_practiceRecordListWidget = new QListWidget;
  m_practiceRecordListWidget->setObjectName("m_practiceRecordListWidget");

  // 创建练习设置布局
  QVBoxLayout* practiceOptionsLayout = createPracticeOptionsLayout();

  // 创建布局
  QVBoxLayout* practiceRecordLayout = new QVBoxLayout;
  practiceRecordLayout->addWidget(m_practiceRecordListWidget);
  practiceRecordLayout->addLayout(practiceOptionsLayout);

  QGroupBox* practiceRecordGroup = new QGroupBox;
  practiceRecordGroup->setObjectName("practiceRecordGroup");
  practiceRecordGroup->setLayout(practiceRecordLayout);

  return practiceRecordGroup;
}

QVBoxLayout* WordMemorizer::createPracticeOptionsLayout() {
  QVBoxLayout* practiceOptionsLayout = new QVBoxLayout;

  QHBoxLayout* modeLayout = new QHBoxLayout;
  m_practiceModeCombo = new QComboBox;
  m_practiceModeCombo->setObjectName("m_practiceModeCombo");
  m_practiceModeCombo->addItem("给出单词，填写释义");
  m_practiceModeCombo->addItem("给出释义，填写单词");

  m_wordCountSpin = new QSpinBox;
  m_wordCountSpin->setObjectName("m_wordCountSpin");
  m_wordCountSpin->setRange(1, std::numeric_limits<int>::max());
  m_wordCountSpin->setValue(5);

  modeLayout->addWidget(m_practiceModeCombo);
  modeLayout->addWidget(m_wordCountSpin);
  practiceOptionsLayout->addLayout(modeLayout);

  m_startPracticeButton = new QPushButton("开始练习");
  m_startPracticeButton->setObjectName("m_startPracticeButton");
  practiceOptionsLayout->addWidget(m_startPracticeButton);

  return practiceOptionsLayout;
}

QGroupBox* WordMemorizer::createPracticeGroup() {
  QGroupBox* practiceGroup = new QGroupBox;
  practiceGroup->setObjectName("practiceGroup");

  QVBoxLayout* practiceGroupLayout = new QVBoxLayout;

  // 计时器设置
  m_practiceTimer = new QTimer(this);
  m_practiceTimer->setInterval(1000);
  m_practiceTimerLabel = new QLabel("00:00", this);
  m_practiceTimerLabel->setObjectName("m_practiceTimerLabel");
  m_practiceTimerLabel->setAlignment(Qt::AlignCenter);
  practiceGroupLayout->addWidget(m_practiceTimerLabel);

  // 题目和答案区域
  m_questionLabel = new QLabel("题目将在这里显示");
  m_questionLabel->setObjectName("m_questionLabel");
  m_questionLabel->setAlignment(Qt::AlignCenter);
  practiceGroupLayout->addWidget(m_questionLabel);

  m_answerEdit = new QLineEdit;
  m_answerEdit->setObjectName("m_answerEdit");
  m_submitButton = new QPushButton("提交答案");
  m_submitButton->setObjectName("m_submitButton");
  m_submitButton->setEnabled(false);

  QHBoxLayout* answerLayout = new QHBoxLayout;
  answerLayout->addWidget(m_answerEdit);
  answerLayout->addWidget(m_submitButton);
  practiceGroupLayout->addLayout(answerLayout);

  // 准确率和练习列表
  m_practiceAccuracyLabel = new QLabel("");
  m_practiceAccuracyLabel->setObjectName("m_practiceAccuracyLabel");
  m_practiceAccuracyLabel->setAlignment(Qt::AlignLeft);
  practiceGroupLayout->addWidget(m_practiceAccuracyLabel);

  m_practiceWordsListWidget = new QListWidget;
  m_practiceWordsListWidget->setObjectName("m_practiceWordsListWidget");
  m_practiceWordsListWidget->setItemDelegate(new PracticeResultDelegate(this));
  practiceGroupLayout->addWidget(m_practiceWordsListWidget);

  // 退出按钮
  m_exitPracticeButton = new QPushButton("退出练习");
  m_exitPracticeButton->setObjectName("m_exitPracticeButton");
  practiceGroupLayout->addWidget(m_exitPracticeButton);

  practiceGroup->setLayout(practiceGroupLayout);
  practiceGroup->hide();

  return practiceGroup;
}

void WordMemorizer::createEditDialog() {
  m_editDialog = new QDialog(this);
  m_editDialog->setObjectName("m_editDialog");
  m_editDialog->setWindowTitle("编辑单词");
  m_editDialog->setModal(true);
  m_editDialog->resize(400, 300);

  QVBoxLayout* editLayout = new QVBoxLayout(m_editDialog);

  // 单词基本信息
  QFormLayout* wordFormLayout = new QFormLayout();
  m_editWordInput = new QLineEdit();
  m_editWordInput->setObjectName("m_editWordInput");
  m_editPrioritySpin = new QSpinBox;
  m_editPrioritySpin->setObjectName("m_editPrioritySpin");
  m_editPrioritySpin->setRange(0, 100);
  m_editPrioritySpin->setValue(1);

  wordFormLayout->addRow("单词:", m_editWordInput);
  wordFormLayout->addRow("优先级:", m_editPrioritySpin);
  editLayout->addLayout(wordFormLayout);

  // 释义编辑区域
  QGroupBox* defGroup = new QGroupBox("释义");
  defGroup->setObjectName("defGroup");
  QVBoxLayout* defLayout = new QVBoxLayout(defGroup);

  QHBoxLayout* defInputLayout = new QHBoxLayout();
  m_editDefinitionInput = new QLineEdit();
  m_editDefinitionInput->setObjectName("m_editDefinitionInput");
  m_editAddDefButton = new QPushButton("添加释义");
  m_editAddDefButton->setObjectName("m_editAddDefButton");

  defInputLayout->addWidget(m_editDefinitionInput);
  defInputLayout->addWidget(m_editAddDefButton);
  defLayout->addLayout(defInputLayout);

  m_editDefinitionList = new QListWidget();
  m_editDefinitionList->setObjectName("m_editDefinitionList");
  m_editDefinitionList->setMinimumHeight(120);
  defLayout->addWidget(m_editDefinitionList);

  m_editRemoveDefButton = new QPushButton("删除选中释义");
  m_editRemoveDefButton->setObjectName("m_editRemoveDefButton");
  defLayout->addWidget(m_editRemoveDefButton);

  editLayout->addWidget(defGroup);

  // 操作按钮
  QHBoxLayout* buttonLayout = new QHBoxLayout();
  m_editSaveButton = new QPushButton("保存");
  m_editSaveButton->setObjectName("m_editSaveButton");
  m_editCancelButton = new QPushButton("取消");
  m_editCancelButton->setObjectName("m_editCancelButton");

  buttonLayout->addWidget(m_editSaveButton);
  buttonLayout->addWidget(m_editCancelButton);
  editLayout->addLayout(buttonLayout);
}

WordMemorizer::~WordMemorizer() {
  // 确保在析构时保存数据
  saveDatabase();
}

void WordMemorizer::saveDatabase() {
  std::optional<WordDatabaseError> err = m_db.saveToFile();
  if (err) {
    QMessageBox::critical(this, "错误", err.value().message());
  }
}

void WordMemorizer::updateTimerDisplay() {
  int minutes = m_elapsedSeconds / 60;
  int seconds = m_elapsedSeconds % 60;
  QString timeText = QString("练习时间: %1:%2")
                         .arg(minutes, 2, 10, QLatin1Char('0'))
                         .arg(seconds, 2, 10, QLatin1Char('0'));
  m_practiceTimerLabel->setText(timeText);
}

void WordMemorizer::onPracticeTimerTimeout() {
  m_elapsedSeconds++;
  updateTimerDisplay();
}

void WordMemorizer::syncWordListDataCacheFromDb() {
  const QVector<WordEntry>& words = m_db.getWords();
  m_wordListDataCache.clear();
  for (qsizetype i = 0; i < words.size(); ++i) {
    m_wordListDataCache.append({i, &words[i]});
  }
}

void WordMemorizer::syncAndUpdateWordList() {
  syncWordListDataCacheFromDb();
  updateWordList();
}

void WordMemorizer::updatePracticeRecordConstViews() {
  m_practiceRecordConstViews.clear();
  const QVector<PracticeRecord>& records = m_db.getPracticeRecords();
  for (qsizetype i = 0; i < records.size(); ++i) {
    m_practiceRecordConstViews.append({i, records[i]});
  }
}

void WordMemorizer::updatePracticeRecordList() {
  m_practiceRecordListWidget->clear();
  for (const PracticeRecordConstView& view : m_practiceRecordConstViews) {
    QString recordText =
        QString("%1: 练习时长: %2, 总数: %3, 正确率: %4%")
            .arg(view.record.practiceTime.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(view.record.duration.toString("hh:mm:ss"))
            .arg(view.record.wordCount())
            .arg(view.record.correctRate, 0, 'f', 2);
    QListWidgetItem* item = new QListWidgetItem(recordText);
    QVariant data = static_cast<qulonglong>(view.index);
    item->setData(Qt::UserRole, data);
    m_practiceRecordListWidget->addItem(item);
  }
}

void WordMemorizer::syncAndUpdatePracticeRecordList() {
  updatePracticeRecordConstViews();
  updatePracticeRecordList();
}

void WordMemorizer::loadSearchCompleterModel() {
  const QVector<WordEntry>& words = m_db.getWords();
  QStringList wordList;
  for (const WordEntry& entry : words) {
    wordList << entry.word;
  }
  m_searchModel->setStringList(wordList);
}

void WordMemorizer::updateSearchCompleterModel(const QString& oldWord,
                                               const QString& newWord) {
  if (oldWord.isEmpty()) return;
  if (oldWord == newWord) return;
  QStringList wordList = m_searchModel->stringList();
  int row = wordList.indexOf(oldWord);
  if (row == -1) return;
  QModelIndex idx = m_searchModel->index(row);
  m_searchModel->setData(idx, newWord, Qt::EditRole);
}

bool WordMemorizer::validateWordInput(const QString& word,
                                      const QStringList& definitions) {
  if (word.isEmpty()) {
    QMessageBox::warning(this, "添加失败", "单词不能为空", QMessageBox::Ok);
    return false;
  }

  if (definitions.isEmpty()) {
    QMessageBox::warning(this, "添加失败", "至少添加一个释义", QMessageBox::Ok);
    return false;
  }

  return true;
}

void WordMemorizer::onAddWord() {
  QString word = m_wordEdit->text().trimmed();

  QStringList definitions;
  for (int i = 0; i < m_definitionListWidget->count(); ++i) {
    definitions << m_definitionListWidget->item(i)->text().trimmed();
  }

  if (!validateWordInput(word, definitions)) {
    return;
  }

  QString definition = definitions.join(", ");

  std::optional<WordDatabaseError> err =
      m_db.addWord(word, definition, m_prioritySpin->value());
  if (err) {
    QMessageBox::warning(this, "添加失败", err.value().message(),
                         QMessageBox::Ok);
    return;
  }

  err = m_db.saveToFile();
  if (err) {
    QMessageBox::warning(this, "保存失败", err.value().message(),
                         QMessageBox::Ok);
    return;
  }

  syncAndUpdateWordList();
  m_wordEdit->clear();
  m_definitionListWidget->clear();
  m_definitionEdit->clear();
}

void WordMemorizer::onDeleteWord() {
  QListWidgetItem* current = m_wordListWidget->currentItem();
  if (!current) {
    QMessageBox::warning(this, "删除失败", "请先选择要删除的单词",
                         QMessageBox::Ok);
    return;
  }

  QString word = current->text().split(" - ").first();

  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, "确认删除",
                                QString("确认删除单词 \"%1\" 吗？").arg(word),
                                QMessageBox::Yes | QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    std::optional<WordDatabaseError> err = m_db.removeWord(word);
    if (err) {
      QMessageBox::warning(this, "删除失败", err.value().message(),
                           QMessageBox::Ok);
      return;
    }

    err = m_db.saveToFile();
    if (err) {
      QMessageBox::warning(this, "保存失败", err.value().message(),
                           QMessageBox::Ok);
      return;
    }

    QMessageBox::information(this, "成功", "单词删除成功");
    syncAndUpdateWordList();
  }
}

void WordMemorizer::onStartPractice() {
  m_questionLabel->setText("准备开始练习...");

  int totalCount = m_db.wordCount();
  int requestedCount = m_wordCountSpin->value();

  if (totalCount == 0) {
    QMessageBox::warning(this, "练习失败", "还没有录入任何单词",
                         QMessageBox::Ok);
    return;
  }

  if (requestedCount > totalCount) {
    QMessageBox::warning(
        this, "练习失败",
        QString("练习失败，请求的题目数量 (%1) 超过了单词总数 (%2)")
            .arg(requestedCount)
            .arg(totalCount),
        QMessageBox::Ok);
    return;
  }

  m_currentMode =
      static_cast<PracticeMode>(m_practiceModeCombo->currentIndex());
  m_totalQuestions = requestedCount;
  m_correctCount = 0;
  m_currentIndex = 0;
  m_elapsedSeconds = 0;

  m_practiceWordIndices =
      std::move(radomlySelectWords(m_db.getWords(), requestedCount));

  findChild<QGroupBox*>("practiceGroup")->show();

  m_practiceAccuracyLabel->setText("答对题数\n正确率");

  findChild<QGroupBox*>("wordsGroup")->hide();
  findChild<QGroupBox*>("practiceRecordGroup")->hide();

  m_practiceRecord.practiceTime = QDateTime::currentDateTime();
  m_practiceRecord.correctWords.clear();
  m_practiceRecord.wrongWords.clear();

  m_practiceTimer->start();
  updateTimerDisplay();
  if (m_currentMode == PracticeMode::WordToDefinition) {
    m_practiceRecord.mode = PracticeMode::WordToDefinition;
    startModeWordToDefinition();
  } else {
    m_practiceRecord.mode = PracticeMode::DefinitionToWord;
    startModeDefinitionToWord();
  }
}

void WordMemorizer::startModeWordToDefinition() {
  if (m_currentIndex < m_practiceWordIndices.size()) {
    qsizetype index = m_practiceWordIndices[m_currentIndex];
    const QVector<WordEntry>& words = m_db.getWords();
    const WordEntry& entry = words[index];
    m_questionLabel->setText(
        QString("第 %1 题：%2 ").arg(m_currentIndex + 1).arg(entry.word));
    m_answerEdit->clear();
    m_answerEdit->setFocus();
    m_answerEdit->setReadOnly(false);
    m_submitButton->setEnabled(true);
  } else {
    endPractice();
  }
}

void WordMemorizer::startModeDefinitionToWord() {
  if (m_currentIndex < m_practiceWordIndices.size()) {
    qsizetype index = m_practiceWordIndices[m_currentIndex];
    const QVector<WordEntry>& words = m_db.getWords();
    const WordEntry& entry = words[index];
    m_questionLabel->setText(QString("第 %1 题：%2 ")
                                 .arg(m_currentIndex + 1)
                                 .arg(randomlySelectDefinition(entry)));
    m_answerEdit->clear();
    m_answerEdit->setFocus();
    m_answerEdit->setReadOnly(false);
    m_submitButton->setEnabled(true);
  } else {
    endPractice();
  }
}

void WordMemorizer::onPracticeSubmit() {
  if (m_currentIndex < 0 ||
      m_currentIndex >= static_cast<int>(m_practiceWordIndices.size())) {
    qWarning() << "Invalid practice index:" << m_currentIndex;
    return;
  }
  QString userAnswer = m_answerEdit->text().trimmed();
  qsizetype index = m_practiceWordIndices[m_currentIndex];
  QVector<WordEntry>& words = m_db.getMutWords();
  WordEntry& correct = words[index];
  correct.totalAttempts++;

  bool isCorrect = false;
  if (userAnswer.isEmpty()) {
    isCorrect = false;
  } else if (m_currentMode == PracticeMode::WordToDefinition) {
    isCorrect = correct.definition.contains(userAnswer, Qt::CaseInsensitive);
  } else {
    isCorrect = correct.word.contains(userAnswer, Qt::CaseInsensitive);
  }

  if (isCorrect) {
    m_practiceRecord.correctWords.append(
        {correct.word, correct.definition, userAnswer});
    m_correctCount++;
    m_questionLabel->setText("<font color='green'>回答正确</font>");
  } else {
    m_practiceRecord.wrongWords.append(
        {correct.word, correct.definition, userAnswer});
    correct.totalErrors++;
    m_questionLabel->setText("<font color='red'>回答错误</font>");
  }

  m_submitButton->setEnabled(false);
  m_answerEdit->setReadOnly(true);

  QListWidgetItem* item =
      new QListWidgetItem(correct.word + " - " + correct.definition);
  item->setData(Qt::UserRole, !isCorrect);
  m_practiceWordsListWidget->addItem(item);

  QTimer::singleShot(1000, this, [this]() {
    m_currentIndex++;
    if (m_currentIndex < m_totalQuestions) {
      if (m_currentMode == PracticeMode::WordToDefinition) {
        startModeWordToDefinition();
      } else {
        startModeDefinitionToWord();
      }
      m_answerEdit->setReadOnly(false);
    } else {
      endPractice();
    }
  });
}

void WordMemorizer::endPractice() {
  m_practiceTimer->stop();

  double accuracy = (double)m_correctCount / m_totalQuestions * 100.0;
  QString result = QString("答对题数: %1 / %2\n正确率: %3%")
                       .arg(m_correctCount)
                       .arg(m_totalQuestions)
                       .arg(accuracy, 0, 'f', 1);

  m_practiceRecord.duration = QTime(0, 0, 0).addSecs(m_elapsedSeconds);
  m_practiceRecord.correctRate = accuracy;
  m_db.getMutPracticeRecords().append(m_practiceRecord);
  syncAndUpdatePracticeRecordList();

  m_questionLabel->setText("练习结束");
  m_practiceAccuracyLabel->setText(result);
  m_answerEdit->clear();
  m_answerEdit->setReadOnly(true);
  m_submitButton->setEnabled(false);
}

void WordMemorizer::onExitPractice() {
  m_practiceTimer->stop();
  m_practiceWordsListWidget->clear();

  // 显示单词列表和练习选项，隐藏练习界面
  QGroupBox* practiceGroup = findChild<QGroupBox*>("practiceGroup");
  QGroupBox* wordsGroup = findChild<QGroupBox*>("wordsGroup");
  QGroupBox* practiceRecordGroup = findChild<QGroupBox*>("practiceRecordGroup");

  if (practiceGroup) practiceGroup->hide();
  if (wordsGroup) wordsGroup->show();
  if (practiceRecordGroup) practiceRecordGroup->show();

  // 保存练习后的状态
  saveDatabase();
}

void WordMemorizer::updateWordList() {
  switch (m_wordListSortOrder) {
    default:
    case SortOrder::CreationTimeAsc:
      std::sort(m_wordListDataCache.begin(), m_wordListDataCache.end(),
                [](const std::tuple<qsizetype, const WordEntry*>& a,
                   const std::tuple<qsizetype, const WordEntry*>& b) {
                  return std::get<1>(a)->createdAt < std::get<1>(b)->createdAt;
                });
      break;
    case SortOrder::CreationTimeDesc:
      std::sort(m_wordListDataCache.begin(), m_wordListDataCache.end(),
                [](const std::tuple<qsizetype, const WordEntry*>& a,
                   const std::tuple<qsizetype, const WordEntry*>& b) {
                  return std::get<1>(a)->createdAt > std::get<1>(b)->createdAt;
                });
      break;
    case SortOrder::ModifiedAtAsc:
      std::sort(m_wordListDataCache.begin(), m_wordListDataCache.end(),
                [](const std::tuple<qsizetype, const WordEntry*>& a,
                   const std::tuple<qsizetype, const WordEntry*>& b) {
                  return std::get<1>(a)->modifiedAt <
                         std::get<1>(b)->modifiedAt;
                });
      break;
    case SortOrder::ModifiedAtDesc:
      std::sort(m_wordListDataCache.begin(), m_wordListDataCache.end(),
                [](const std::tuple<qsizetype, const WordEntry*>& a,
                   const std::tuple<qsizetype, const WordEntry*>& b) {
                  return std::get<1>(a)->modifiedAt >
                         std::get<1>(b)->modifiedAt;
                });
      break;
    case SortOrder::PriorityAsc:
      std::sort(m_wordListDataCache.begin(), m_wordListDataCache.end(),
                [](const std::tuple<qsizetype, const WordEntry*>& a,
                   const std::tuple<qsizetype, const WordEntry*>& b) {
                  return std::get<1>(a)->priority < std::get<1>(b)->priority;
                });
      break;
    case SortOrder::PriorityDesc:
      std::sort(m_wordListDataCache.begin(), m_wordListDataCache.end(),
                [](const std::tuple<qsizetype, const WordEntry*>& a,
                   const std::tuple<qsizetype, const WordEntry*>& b) {
                  return std::get<1>(a)->priority > std::get<1>(b)->priority;
                });
      break;
    case SortOrder::AlphabeticalAsc:
      std::sort(m_wordListDataCache.begin(), m_wordListDataCache.end(),
                [](const std::tuple<qsizetype, const WordEntry*>& a,
                   const std::tuple<qsizetype, const WordEntry*>& b) {
                  return std::get<1>(a)->word < std::get<1>(b)->word;
                });
      break;
    case SortOrder::AlphabeticalDesc:
      std::sort(m_wordListDataCache.begin(), m_wordListDataCache.end(),
                [](const std::tuple<qsizetype, const WordEntry*>& a,
                   const std::tuple<qsizetype, const WordEntry*>& b) {
                  return std::get<1>(a)->word > std::get<1>(b)->word;
                });
      break;
  }

  m_wordListWidget->clear();
  for (const auto& [index, entry] : m_wordListDataCache) {
    QListWidgetItem* item = new QListWidgetItem(entry->word);
    QVariant value = static_cast<qulonglong>(index);
    item->setData(Qt::UserRole, value);
    m_wordListWidget->addItem(item);
  }
}

void WordMemorizer::onWordListContextMenuRequested(const QPoint& pos) {
  QListWidgetItem* item = m_wordListWidget->itemAt(pos);
  if (item) {
    m_wordListWidget->setCurrentItem(item);
    m_wordListContextMenu->exec(m_wordListWidget->mapToGlobal(pos));
  }
}

void WordMemorizer::onEditWord() {
  QListWidgetItem* currItem = m_wordListWidget->currentItem();
  if (!currItem) {
    QMessageBox::warning(this, "编辑失败", "请先选择要编辑的单词",
                         QMessageBox::Ok);
    return;
  }

  try {
    qsizetype idx = currItem->data(Qt::UserRole).toULongLong();
    const QVector<WordEntry>& words = m_db.getWords();

    if (idx >= words.size()) {
      QMessageBox::warning(this, "编辑失败", "无效的单词索引", QMessageBox::Ok);
      return;
    }

    const WordEntry& entry = words[idx];

    m_editingOriginalWord = entry.word;
    m_editPrioritySpin->setValue(entry.priority);
    m_editWordInput->setText(entry.word);
    m_editDefinitionList->clear();

    QStringList defs = entry.definition.split(",", Qt::SkipEmptyParts);
    for (const QString& def : defs) {
      m_editDefinitionList->addItem(def.trimmed());
    }

    m_editDefinitionInput->clear();
    m_editDialog->show();
    m_editWordInput->setFocus();
  } catch (const std::exception& e) {
    QMessageBox::warning(this, "编辑失败",
                         QString("发生错误: %1").arg(e.what()),
                         QMessageBox::Ok);
  }
}

void WordMemorizer::onSaveWordEdit() {
  QString newWord = m_editWordInput->text().trimmed();

  QStringList definitions;
  for (int i = 0; i < m_editDefinitionList->count(); ++i) {
    definitions << m_editDefinitionList->item(i)->text().trimmed();
  }

  if (!validateWordInput(newWord, definitions)) {
    return;
  }

  QString newDefinition = definitions.join(", ");

  int newPriority = m_editPrioritySpin->value();

  std::optional<WordDatabaseError> err = m_db.updateWord(
      m_editingOriginalWord, newWord, newDefinition, newPriority);
  if (err) {
    QMessageBox::warning(this, "修改失败", err.value().message());
    return;
  }

  err = m_db.saveToFile();
  if (err) {
    QMessageBox::warning(this, "保存失败", err.value().message());
    return;
  }

  syncAndUpdateWordList();
  m_editDialog->hide();
}

void WordMemorizer::onCancelWordEdit() { m_editDialog->hide(); }

void WordMemorizer::initializeDatabase() {
  // 此方法已经在构造函数中通过loadFromFile实现
  // 保持方法定义以符合头文件声明
}

void WordMemorizer::setupStyle() {
  QFile styleFile(STYLE_FILE_PATH);

  if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
    QTextStream styleStream(&styleFile);
    QString styleSheet = styleStream.readAll();
    setStyleSheet(styleSheet);
    m_practiceRecordDisplayer->setupStyle(styleSheet);
    styleFile.close();
  } else {
    QMessageBox::critical(this, "错误", "无法加载样式表");
  }
}

void PracticeRecordDisplayer::setupUi() {
  QWidget* centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
  QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

  m_practiceTimeLabel = new QLabel("");
  m_practiceDurationLabel = new QLabel("");
  m_practiceCorrectRateLabel = new QLabel("");
  m_practiceModeLabel = new QLabel("");
  m_wrongWordsList = new QListWidget();
  QLabel* wrongWordsListLabel = new QLabel("错误单词:");
  m_wrongWordsList->setItemDelegate(new PracticeResultDelegate(this));

  mainLayout->addWidget(m_practiceTimeLabel);
  mainLayout->addWidget(m_practiceDurationLabel);
  mainLayout->addWidget(m_practiceCorrectRateLabel);
  mainLayout->addWidget(m_practiceModeLabel);
  mainLayout->addWidget(wrongWordsListLabel);
  mainLayout->addWidget(m_wrongWordsList);
}

void PracticeRecordDisplayer::setupStyle(const QString& styleSheet) {
  setStyleSheet(styleSheet);
}

void PracticeRecordDisplayer::displayRecord(const PracticeRecord& record) {
  m_practiceTimeLabel->setText(
      QString("练习时间: %1")
          .arg(record.practiceTime.toString("yyyy-MM-dd hh:mm:ss")));
  m_practiceDurationLabel->setText(
      QString("练习时长: %1").arg(record.duration.toString("hh:mm:ss")));
  m_practiceCorrectRateLabel->setText(
      QString("正确率: %1%").arg(record.correctRate));
  m_practiceModeLabel->setText(
      QString("练习模式: %1")
          .arg(record.mode == PracticeMode::WordToDefinition ? "单词到定义"
                                                             : "定义到单词"));

  m_wrongWordsList->clear();
  for (const PracticeEntry& entry : record.wrongWords) {
    QString text = QString("%1 - %2\n\n你的答案: %3")
                       .arg(entry.word)
                       .arg(entry.definition)
                       .arg(entry.userAnswer);
    QListWidgetItem* item = new QListWidgetItem(text);
    item->setData(Qt::UserRole, QVariant::fromValue(true));
    m_wrongWordsList->addItem(item);
  }
}
