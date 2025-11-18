#ifndef WORDMEMORIZER_HPP
#define WORDMEMORIZER_HPP

#include <QCloseEvent>
#include <QComboBox>
#include <QCompleter>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>

#include "worddatabase.hpp"

class PracticeResultDelegate : public QStyledItemDelegate {
  Q_OBJECT

 public:
  explicit PracticeResultDelegate(QObject* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;
};

class PracticeRecordDisplayer : public QMainWindow {
  Q_OBJECT

 public:
  explicit PracticeRecordDisplayer(QWidget* parent = nullptr)
      : QMainWindow(parent) {
    setWindowTitle("练习记录详情");
    resize(400, 500);
    hide();
  }

  void setupUi();
  void setupStyle(const QString& styleSheet);

  void displayRecord(const PracticeRecord& record);

 protected:
  void closeEvent(QCloseEvent* event) override {
    event->ignore();
    hide();
  }

 private:
  QLabel* m_practiceTimeLabel = nullptr;
  QLabel* m_practiceDurationLabel = nullptr;
  QLabel* m_practiceCorrectRateLabel = nullptr;
  QLabel* m_practiceModeLabel = nullptr;
  QListWidget* m_wrongWordsList = nullptr;
};

/**
 * @brief 单词记忆应用主窗口类
 */
class WordMemorizer : public QMainWindow {
  Q_OBJECT

 public:
  explicit WordMemorizer(QWidget* parent = nullptr);
  ~WordMemorizer() override;

  /**
   * @brief 排序方式枚举
   */
  enum class SortOrder {
    CreationTimeAsc = 0,  ///< 按创建时间升序
    CreationTimeDesc,     ///< 按创建时间降序
    ModifiedAtAsc,        ///< 按修改时间升序
    ModifiedAtDesc,       ///< 按修改时间降序
    PriorityAsc,          ///< 按优先级升序
    PriorityDesc,         ///< 按优先级降序
    AlphabeticalAsc,      ///< 按字母升序
    AlphabeticalDesc,     ///< 按字母降序
  };

 private slots:
  // 单词管理相关槽函数
  void onAddWord();
  void onDeleteWord();
  void onEditWord();
  void onSaveWordEdit();
  void onCancelWordEdit();

  // 练习相关槽函数
  void onStartPractice();
  void onPracticeSubmit();
  void onExitPractice();

  // 列表和搜索相关槽函数
  void onWordListContextMenuRequested(const QPoint& pos);
  void onCurrentItemChanged(QListWidgetItem* current,
                            QListWidgetItem* previous);
  void onSortOrderChanged();
  void onSearchEditReturnPressed();

  // 练习记录相关槽函数
  void onPracticeRecordDoubleClicked(QListWidgetItem* item);

  // 定时器槽函数
  void onPracticeTimerTimeout();

 private:
  // 初始化相关函数
  void initializeDatabase();
  void setupUi();
  void setupStyle();
  void setupConnections();

  // UI组件创建函数
  QGroupBox* createWordsGroup();
  QFormLayout* createWordInputLayout();
  QVBoxLayout* createWordListLayout();
  void setupSearchCompleter();
  void setupContextMenu();
  QGroupBox* createPracticeRecordGroup();
  QVBoxLayout* createPracticeOptionsLayout();
  QGroupBox* createPracticeGroup();
  void createEditDialog();

  // 数据管理函数
  void syncWordListDataCacheFromDb();
  void syncAndUpdateWordList();
  void updateWordList();
  void updatePracticeRecordConstViews();
  void updatePracticeRecordList();
  void syncAndUpdatePracticeRecordList();

  // 搜索相关函数
  void loadSearchCompleterModel();
  void updateSearchCompleterModel(const QString& oldWord,
                                  const QString& newWord);

  // 练习模式相关函数
  void startModeWordToDefinition();
  void startModeDefinitionToWord();
  void endPractice();
  void updateTimerDisplay();

  // 验证函数
  bool validateWordInput(const QString& word, const QStringList& definitions);

  // 数据持久化函数
  void saveDatabase();

  // 数据库对象
  WordDatabase m_db;

  // 单词录入相关UI组件
  QLineEdit* m_wordEdit;
  QLineEdit* m_definitionEdit;
  QSpinBox* m_prioritySpin;
  QListWidget* m_definitionListWidget;
  QPushButton* m_addDefinitionButton;
  QPushButton* m_removeDefinitionButton;
  QPushButton* m_addButton;
  QPushButton* m_deleteButton;

  // 单词列表相关UI组件
  bool m_isSearching;
  QPushButton* m_searchCancelButton;
  QLineEdit* m_searchEdit;
  QCompleter* m_searchCompleter;
  QStringListModel* m_searchModel;
  QVector<std::tuple<qsizetype, const WordEntry*>> m_wordListDataCache;
  QComboBox* m_wordListSortOrderCombo;
  SortOrder m_wordListSortOrder;
  QListWidget* m_wordListWidget;

  // 上下文菜单相关
  QMenu* m_wordListContextMenu;
  QAction* m_editWordAction;
  QAction* m_deleteWordAction;

  // 编辑对话框相关
  QDialog* m_editDialog;
  QLineEdit* m_editWordInput;
  QSpinBox* m_editPrioritySpin;
  QListWidget* m_editDefinitionList;
  QLineEdit* m_editDefinitionInput;
  QPushButton* m_editAddDefButton;
  QPushButton* m_editRemoveDefButton;
  QPushButton* m_editSaveButton;
  QPushButton* m_editCancelButton;
  QString m_editingOriginalWord;

  // 练习记录相关
  QListWidget* m_practiceRecordListWidget;
  QVector<PracticeRecordConstView> m_practiceRecordConstViews;
  PracticeRecordDisplayer* m_practiceRecordDisplayer;

  // 练习设置相关
  QPushButton* m_startPracticeButton;
  QComboBox* m_practiceModeCombo;
  QSpinBox* m_wordCountSpin;

  // 练习界面相关
  QLabel* m_questionLabel;
  QLineEdit* m_answerEdit;
  QPushButton* m_submitButton;
  QVector<qsizetype> m_practiceWordIndices;
  QLabel* m_practiceTimerLabel;
  QTimer* m_practiceTimer;
  QPushButton* m_exitPracticeButton;
  QLabel* m_practiceAccuracyLabel;
  QListWidget* m_practiceWordsListWidget;

  // 练习状态变量
  qint64 m_elapsedSeconds;
  int m_currentIndex;
  int m_correctCount;
  int m_totalQuestions;
  PracticeMode m_currentMode;
  PracticeRecord m_practiceRecord;
};

#endif  // WORDMEMORIZER_HPP
