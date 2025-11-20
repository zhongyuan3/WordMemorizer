#ifndef WORDMEMORIZER_H
#define WORDMEMORIZER_H

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

#include "worddatabase.h"

class PracticeResultDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit PracticeResultDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

class PracticeRecordDisplayer : public QMainWindow {
    Q_OBJECT

public:
    explicit PracticeRecordDisplayer(QWidget *parent = nullptr)
        : QMainWindow(parent) {
        setWindowTitle("练习记录详情");
        resize(400, 500);
        hide();
    }

    void setupUi();
    void setupStyle(const QString &styleSheet);

    void displayRecord(const PracticeRecord &record);

protected:
    void closeEvent(QCloseEvent *event) override {
        event->ignore();
        hide();
    }

private:
    QLabel *m_practiceTimeLabel = nullptr;
    QLabel *m_practiceDurationLabel = nullptr;
    QLabel *m_practiceCorrectRateLabel = nullptr;
    QLabel *m_practiceModeLabel = nullptr;
    QListWidget *m_wrongWordsList = nullptr;
};

enum class SortOrder {
    CreationTimeAsc = 0,
    CreationTimeDesc,
    ModifiedAtAsc,
    ModifiedAtDesc,
    PriorityAsc,
    PriorityDesc,
    AlphabeticalAsc,
    AlphabeticalDesc,
};

class WordMemorizer : public QMainWindow {
    Q_OBJECT

public:
    explicit WordMemorizer(QWidget *parent = nullptr);
    ~WordMemorizer() override;

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
    void onWordListContextMenuRequested(const QPoint &pos);
    void onCurrentItemChanged(QListWidgetItem *current,
                              QListWidgetItem *previous);
    void onSortOrderChanged();
    void onSearchEditReturnPressed();

    // 练习记录相关槽函数
    void onPracticeRecordDoubleClicked(QListWidgetItem *item);

    // 定时器槽函数
    void onPracticeTimerTimeout();

private:
    // 初始化相关函数
    void initializeDatabase();
    void setupUi();
    void setupStyle();
    void setupConnections();

    // UI组件创建函数
    QGroupBox *createWordsGroup();
    QFormLayout *createWordInputLayout();
    QVBoxLayout *createWordListLayout();
    void setupSearchCompleter();
    void setupContextMenu();
    QGroupBox *createPracticeRecordGroup();
    QVBoxLayout *createPracticeOptionsLayout();
    QGroupBox *createPracticeGroup();
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
    void updateSearchCompleterModel(const QString &oldWord,
                                    const QString &newWord);

    // 练习模式相关函数
    void startModeWordToDefinition();
    void startModeDefinitionToWord();
    void endPractice();
    void updateTimerDisplay();

    // 验证函数
    bool validateWordInput(const QString &word, const QStringList &definitions);

    // 数据持久化函数
    void saveDatabase();

    // 数据库对象
    WordDatabase m_db;

    // 单词录入相关UI组件
    QLineEdit *m_wordEdit = nullptr;
    QLineEdit *m_definitionEdit = nullptr;
    QSpinBox *m_prioritySpin = nullptr;
    QListWidget *m_definitionListWidget = nullptr;
    QPushButton *m_addDefinitionButton = nullptr;
    QPushButton *m_removeDefinitionButton = nullptr;
    QPushButton *m_addButton = nullptr;
    QPushButton *m_deleteButton = nullptr;

    // 单词列表相关UI组件
    bool m_isSearching = false;
    QPushButton *m_searchCancelButton = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QCompleter *m_searchCompleter = nullptr;
    QStringListModel *m_searchModel = nullptr;
    QVector<WordEntryConstView> m_wordListDataCache;
    QComboBox *m_wordListSortOrderCombo = nullptr;
    SortOrder m_wordListSortOrder = SortOrder::AlphabeticalAsc;
    QListWidget *m_wordListWidget = nullptr;

    // 上下文菜单相关
    QMenu *m_wordListContextMenu = nullptr;
    QAction *m_editWordAction = nullptr;
    QAction *m_deleteWordAction = nullptr;

    // 编辑对话框相关
    QDialog *m_editDialog = nullptr;
    QLineEdit *m_editWordInput = nullptr;
    QSpinBox *m_editPrioritySpin = nullptr;
    QListWidget *m_editDefinitionList = nullptr;
    QLineEdit *m_editDefinitionInput = nullptr;
    QPushButton *m_editAddDefButton = nullptr;
    QPushButton *m_editRemoveDefButton = nullptr;
    QPushButton *m_editSaveButton = nullptr;
    QPushButton *m_editCancelButton = nullptr;
    QString m_editingOriginalWord;

    // 练习记录相关
    QListWidget *m_practiceRecordListWidget = nullptr;
    QVector<PracticeRecordConstView> m_practiceRecordConstViews;
    PracticeRecordDisplayer *m_practiceRecordDisplayer = nullptr;

    // 练习设置相关
    QPushButton *m_startPracticeButton = nullptr;
    QComboBox *m_practiceModeCombo = nullptr;
    QSpinBox *m_wordCountSpin = nullptr;

    // 练习界面相关
    QLabel *m_questionLabel = nullptr;
    QLineEdit *m_answerEdit = nullptr;
    QPushButton *m_submitButton = nullptr;
    QVector<qsizetype> m_practiceWordIndices;
    QLabel *m_practiceTimerLabel = nullptr;
    QTimer *m_practiceTimer = nullptr;
    QPushButton *m_exitPracticeButton = nullptr;
    QLabel *m_practiceAccuracyLabel = nullptr;
    QListWidget *m_practiceWordsListWidget = nullptr;

    // 练习状态变量
    qint64 m_elapsedSeconds = 0;
    int m_currentIndex = -1;
    int m_correctCount = 0;
    int m_totalQuestions = 0;
    PracticeMode m_currentMode = PracticeMode::WordToDefinition;
    PracticeRecord m_practiceRecord;
};

#endif
