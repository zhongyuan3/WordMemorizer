#include "wordmemorizer.hpp"

#include <QApplication>
#include <QDir>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

#define STYLE_FILE_PATH ":/styles/main.qss"

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
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists() && !dir.mkpath(".")) {
        QMessageBox::critical(this, "错误", "创建数据目录失败");
        return;
    }

    QString dataFile = dir.absoluteFilePath("words.json");
    m_db = WordDatabase(dataFile);
    std::optional<WordDatabaseError> err = m_db.loadFromFile();
    if (err) {
        QMessageBox::critical(this, "错误", err.value());
    }

    setupUi();
    setupStyle();
    syncAndUpdateWordList();
    loadSearchCompleterModel();
}

void WordMemorizer::onCurrentItemChanged(QListWidgetItem* curr, QListWidgetItem* prev) {
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
    m_wordListSortOrder = static_cast<SortOrder>(m_wordListSortOrderCombo->currentIndex());
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

void WordMemorizer::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 单词录入
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

    // 单词列表
    m_wordListWidget = new QListWidget;
    m_wordListWidget->setObjectName("m_wordListWidget");
    // 搜索框
    m_searchCancelButton = new QPushButton("取消");
    m_searchCancelButton->setObjectName("m_searchCancelButton");
    m_searchCancelButton->setHidden(true);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setObjectName("m_searchEdit");
    m_searchEdit->setClearButtonEnabled(true);
    QHBoxLayout* searchLayout = new QHBoxLayout;
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchCancelButton);
    m_searchCompleter = new QCompleter(this);
    m_searchCompleter->setObjectName("m_searchCompleter");
    m_searchCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_searchCompleter->setFilterMode(Qt::MatchContains);
    m_searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_searchModel = new QStringListModel(this);
    m_searchCompleter->setModel(m_searchModel);
    m_searchEdit->setCompleter(m_searchCompleter);

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

    // 单词列表排序方式
    m_wordListSortOrderCombo = new QComboBox;
    m_wordListSortOrderCombo->setObjectName("m_wordListSortOrderCombo");
    m_wordListSortOrderCombo->addItem("按创建时间升序");
    m_wordListSortOrderCombo->addItem("按创建时间降序");
    m_wordListSortOrderCombo->addItem("按修改时间升序");
    m_wordListSortOrderCombo->addItem("按修改时间降序");
    m_wordListSortOrderCombo->addItem("按优先级升序");
    m_wordListSortOrderCombo->addItem("按优先级降序");
    QVBoxLayout* wordListLayout = new QVBoxLayout;
    wordListLayout->addLayout(searchLayout);
    wordListLayout->addWidget(m_wordListSortOrderCombo);
    wordListLayout->addWidget(m_wordListWidget);
    // 为单词列表添加上下文菜单
    m_editWordAction = new QAction("编辑单词", this);
    m_deleteWordAction = new QAction("删除单词", this);
    m_wordListContextMenu = new QMenu(this);
    m_wordListContextMenu->setObjectName("m_wordListContextMenu");
    m_wordListContextMenu->addAction(m_editWordAction);
    m_wordListContextMenu->addAction(m_deleteWordAction);
    m_wordListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_editDialog = new QDialog(this);
    m_editDialog->setObjectName("m_editDialog");
    m_editDialog->setWindowTitle("编辑单词");
    m_editDialog->setModal(true);
    m_editDialog->resize(400, 300);
    QVBoxLayout* editLayout = new QVBoxLayout(m_editDialog);
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
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_editSaveButton = new QPushButton("保存");
    m_editSaveButton->setObjectName("m_editSaveButton");
    m_editCancelButton = new QPushButton("取消");
    m_editCancelButton->setObjectName("m_editCancelButton");
    buttonLayout->addWidget(m_editSaveButton);
    buttonLayout->addWidget(m_editCancelButton);
    editLayout->addLayout(buttonLayout);

    QGroupBox* wordsGroup = new QGroupBox;
    wordsGroup->setObjectName("wordsGroup");
    QHBoxLayout* wordGroupLayout = new QHBoxLayout;
    wordGroupLayout->addLayout(wordInputLayout);
    wordGroupLayout->addLayout(wordListLayout);
    wordsGroup->setLayout(wordGroupLayout);

    QGroupBox* practiceOptionsGroup = new QGroupBox;
    practiceOptionsGroup->setObjectName("practiceOptionsGroup");
    QVBoxLayout* practiceLayout = new QVBoxLayout;
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
    practiceLayout->addLayout(modeLayout);
    m_startPracticeButton = new QPushButton("开始练习");
    m_startPracticeButton->setObjectName("m_startPracticeButton");
    practiceLayout->addWidget(m_startPracticeButton);
    practiceOptionsGroup->setLayout(practiceLayout);

    QGroupBox* practiceGroup = new QGroupBox;
    practiceGroup->setObjectName("practiceGroup");
    QVBoxLayout* practiceGroupLayout = new QVBoxLayout;

    m_practiceTimer = new QTimer(this);
    m_practiceTimer->setInterval(1000);
    m_practiceTimerLabel = new QLabel("00:00", this);
    m_practiceTimerLabel->setObjectName("m_practiceTimerLabel");
    m_practiceTimerLabel->setAlignment(Qt::AlignCenter);
    practiceGroupLayout->addWidget(m_practiceTimerLabel);

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

    m_practiceAccuracyLabel = new QLabel("");
    m_practiceAccuracyLabel->setObjectName("m_practiceAccuracyLabel");
    m_practiceAccuracyLabel->setAlignment(Qt::AlignLeft);
    practiceGroupLayout->addWidget(m_practiceAccuracyLabel);

    m_practiceWordsListWidget = new QListWidget;
    m_practiceWordsListWidget->setObjectName("m_practiceWordsListWidget");
    m_practiceWordsListWidget->setItemDelegate(new PracticeResultDelegate(this));
    practiceGroupLayout->addWidget(m_practiceWordsListWidget);

    m_exitPracticeButton = new QPushButton("退出练习");
    m_exitPracticeButton->setObjectName("m_exitPracticeButton");
    practiceGroupLayout->addWidget(m_exitPracticeButton);

    practiceGroup->setLayout(practiceGroupLayout);

    practiceGroup->hide();

    mainLayout->addWidget(wordsGroup);
    mainLayout->addWidget(practiceOptionsGroup);
    mainLayout->addWidget(practiceGroup);

    connect(m_searchCancelButton, &QPushButton::clicked, this, [this]() {
        m_searchEdit->clear();
        onSearchEditReturnPressed();
    });
    connect(m_searchEdit, &QLineEdit::returnPressed, this,
            &WordMemorizer::onSearchEditReturnPressed);
    connect(m_wordListSortOrderCombo, &QComboBox::currentIndexChanged, this,
            &WordMemorizer::onSortOrderChanged);
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
            delete m_definitionListWidget->takeItem(m_definitionListWidget->row(item));
        }
    });
    connect(m_addButton, &QPushButton::clicked, this, &WordMemorizer::onAddWord);
    connect(m_deleteButton, &QPushButton::clicked, this, &WordMemorizer::onDeleteWord);
    connect(m_startPracticeButton, &QPushButton::clicked, this, &WordMemorizer::onStartPractice);
    connect(m_answerEdit, &QLineEdit::returnPressed, this, &WordMemorizer::onPracticeSubmit);
    connect(m_submitButton, &QPushButton::clicked, this, &WordMemorizer::onPracticeSubmit);
    connect(m_exitPracticeButton, &QPushButton::clicked, this, &WordMemorizer::onExitPractice);
    connect(m_practiceTimer, &QTimer::timeout, this, [this]() {
        m_elapsedSeconds++;
        updateTimerDisplay();
    });
    connect(m_wordListWidget, &QListWidget::customContextMenuRequested, this,
            &WordMemorizer::onWordListContextMenuRequested);
    connect(m_wordListWidget, &QListWidget::currentItemChanged, this,
            &WordMemorizer::onCurrentItemChanged);
    connect(m_editWordAction, &QAction::triggered, this, &WordMemorizer::onEditWord);
    connect(m_deleteWordAction, &QAction::triggered, this, &WordMemorizer::onDeleteWord);
    connect(m_editAddDefButton, &QPushButton::clicked, this, [this]() {
        QString input = m_editDefinitionInput->text().trimmed();
        if (input.isEmpty()) return;
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
    connect(m_editSaveButton, &QPushButton::clicked, this, &WordMemorizer::onSaveWordEdit);
    connect(m_editCancelButton, &QPushButton::clicked, this, &WordMemorizer::onCancelWordEdit);
    connect(m_editDefinitionInput, &QLineEdit::returnPressed, m_editAddDefButton,
            &QPushButton::click);
    connect(qApp, &QApplication::aboutToQuit, this, [this]() {
        std::optional<WordDatabaseError> err = m_db.saveToFile();
        if (err) QMessageBox::critical(this, "错误", err.value());
    });
}

void WordMemorizer::updateTimerDisplay() {
    int minutes = m_elapsedSeconds / 60;
    int seconds = m_elapsedSeconds % 60;
    QString timeText = QString("练习时间: %1:%2")
                           .arg(minutes, 2, 10, QLatin1Char('0'))
                           .arg(seconds, 2, 10, QLatin1Char('0'));
    m_practiceTimerLabel->setText(timeText);
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

void WordMemorizer::loadSearchCompleterModel() {
    const QVector<WordEntry>& words = m_db.getWords();
    QStringList wordList;
    for (const WordEntry& entry : words) {
        wordList << entry.word;
    }
    m_searchModel->setStringList(wordList);
}

void WordMemorizer::updateSearchCompleterModel(const QString& oldWord, const QString& newWord) {
    if (oldWord.isEmpty()) return;
    if (oldWord == newWord) return;
    QStringList wordList = m_searchModel->stringList();
    int row = wordList.indexOf(oldWord);
    if (row == -1) return;
    QModelIndex idx = m_searchModel->index(row);
    m_searchModel->setData(idx, newWord, Qt::EditRole);
}

void WordMemorizer::onAddWord() {
    QString word = m_wordEdit->text().trimmed();

    if (word.isEmpty()) {
        QMessageBox::warning(this, "添加失败", "单词和释义不能为空", QMessageBox::Ok);
        return;
    }

    QStringList definitions;
    for (int i = 0; i < m_definitionListWidget->count(); ++i) {
        definitions << m_definitionListWidget->item(i)->text().trimmed();
    }

    if (definitions.isEmpty()) {
        QMessageBox::warning(this, "添加失败", "至少添加一个释义", QMessageBox::Ok);
        return;
    }

    QString definition = definitions.join(", ");

    std::optional<WordDatabaseError> err = m_db.addWord(word, definition, m_prioritySpin->value());
    if (err) {
        QMessageBox::warning(this, "添加失败", err.value(), QMessageBox::Ok);
        return;
    }

    err = m_db.saveToFile();
    if (err) {
        QMessageBox::warning(this, "保存失败", err.value(), QMessageBox::Ok);
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
        QMessageBox::warning(this, "删除失败", "请先选择要删除的单词", QMessageBox::Ok);
        return;
    }

    QString word = current->text().split(" - ").first();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除", QString("确认删除单词 \"%1\" 吗？").arg(word),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        std::optional<WordDatabaseError> err = m_db.removeWord(word);
        if (err) {
            QMessageBox::warning(this, "删除失败", err.value(), QMessageBox::Ok);
            return;
        }

        err = m_db.saveToFile();
        if (err) {
            QMessageBox::warning(this, "保存失败", err.value(), QMessageBox::Ok);
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
        QMessageBox::warning(this, "练习失败", "还没有录入任何单词", QMessageBox::Ok);
        return;
    }

    if (requestedCount > totalCount) {
        QMessageBox::warning(this, "练习失败",
                             QString("练习失败，请求的题目数量 (%1) 超过了单词总数 (%2)")
                                 .arg(requestedCount)
                                 .arg(totalCount),
                             QMessageBox::Ok);
        return;
    }

    m_currentMode = static_cast<PracticeMode>(m_practiceModeCombo->currentIndex());
    m_totalQuestions = requestedCount;
    m_correctCount = 0;
    m_currentIndex = 0;
    m_elapsedSeconds = 0;

    const QVector<WordEntry>& words = m_db.getWords();
    m_practiceWordIndices.clear();
    for (qsizetype i = 0; i < words.size(); ++i) {
        m_practiceWordIndices.append(i);
    }
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::shuffle(m_practiceWordIndices.begin(), m_practiceWordIndices.end(), gen);

    findChild<QGroupBox*>("practiceGroup")->show();

    m_practiceAccuracyLabel->setText("答对题数\n正确率");

    findChild<QGroupBox*>("wordsGroup")->hide();
    findChild<QGroupBox*>("practiceOptionsGroup")->hide();

    m_practiceTimer->start();
    updateTimerDisplay();
    if (m_currentMode == PracticeMode::WordToDefinition) {
        startModeWordToDef();
    } else {
        startModeDefToWord();
    }
}

void WordMemorizer::startModeWordToDef() {
    if (m_currentIndex < m_practiceWordIndices.size()) {
        qsizetype index = m_practiceWordIndices[m_currentIndex];
        const QVector<WordEntry>& words = m_db.getWords();
        const WordEntry& entry = words[index];
        m_questionLabel->setText(QString("第 %1 题：%2 ").arg(m_currentIndex + 1).arg(entry.word));
        m_answerEdit->clear();
        m_answerEdit->setFocus();
        m_answerEdit->setReadOnly(false);
        m_submitButton->setEnabled(true);
    } else {
        endPractice();
    }
}

static QString randomDefinition(const WordEntry& entry) {
    QStringList parts = entry.definition.split(",");
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::shuffle(parts.begin(), parts.end(), gen);
    return parts.front();
}

void WordMemorizer::startModeDefToWord() {
    if (m_currentIndex < m_practiceWordIndices.size()) {
        qsizetype index = m_practiceWordIndices[m_currentIndex];
        const QVector<WordEntry>& words = m_db.getWords();
        const WordEntry& entry = words[index];
        m_questionLabel->setText(
            QString("第 %1 题：%2 ").arg(m_currentIndex + 1).arg(randomDefinition(entry)));
        m_answerEdit->clear();
        m_answerEdit->setFocus();
        m_answerEdit->setReadOnly(false);
        m_submitButton->setEnabled(true);
    } else {
        endPractice();
    }
}

void WordMemorizer::onPracticeSubmit() {
    QString userAnswer = m_answerEdit->text().trimmed();
    qsizetype index = m_practiceWordIndices[m_currentIndex];
    const QVector<WordEntry>& words = m_db.getWords();
    const WordEntry& correct = words[index];

    bool isCorrect = false;
    if (userAnswer.isEmpty()) {
        isCorrect = false;
    } else if (m_currentMode == PracticeMode::WordToDefinition) {
        isCorrect = correct.definition.contains(userAnswer, Qt::CaseInsensitive);
    } else {
        isCorrect = correct.word.contains(userAnswer, Qt::CaseInsensitive);
    }

    if (isCorrect) {
        m_correctCount++;
        m_questionLabel->setText("<font color='green'>回答正确</font>");
    } else {
        m_questionLabel->setText("<font color='red'>回答错误</font>");
    }

    m_submitButton->setEnabled(false);
    m_answerEdit->setReadOnly(true);

    QListWidgetItem* item = new QListWidgetItem(correct.word + " - " + correct.definition);
    item->setData(Qt::UserRole, !isCorrect);
    m_practiceWordsListWidget->addItem(item);

    QTimer::singleShot(1000, this, [this]() {
        m_currentIndex++;
        if (m_currentIndex < m_totalQuestions) {
            if (m_currentMode == PracticeMode::WordToDefinition) {
                startModeWordToDef();
            } else {
                startModeDefToWord();
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

    m_questionLabel->setText("练习结束");
    m_practiceAccuracyLabel->setText(result);
    m_answerEdit->clear();
    m_answerEdit->setReadOnly(true);
    m_submitButton->setEnabled(false);
}

void WordMemorizer::onExitPractice() {
    m_practiceTimer->stop();
    m_practiceWordsListWidget->clear();
    findChild<QGroupBox*>("practiceGroup")->hide();
    findChild<QGroupBox*>("wordsGroup")->show();
    findChild<QGroupBox*>("practiceOptionsGroup")->show();
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
                          return std::get<1>(a)->modifiedAt < std::get<1>(b)->modifiedAt;
                      });
            break;
        case SortOrder::ModifiedAtDesc:
            std::sort(m_wordListDataCache.begin(), m_wordListDataCache.end(),
                      [](const std::tuple<qsizetype, const WordEntry*>& a,
                         const std::tuple<qsizetype, const WordEntry*>& b) {
                          return std::get<1>(a)->modifiedAt > std::get<1>(b)->modifiedAt;
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
    if (!currItem) return;

    qsizetype idx = currItem->data(Qt::UserRole).toULongLong();
    const WordEntry& entry = m_db.getWords()[idx];

    m_editingOriginalWord = entry.word;
    m_editPrioritySpin->setValue(entry.priority);

    m_editWordInput->setText(entry.word);
    m_editDefinitionList->clear();

    QStringList defs = entry.definition.split(",", Qt::SkipEmptyParts);
    for (const QString& def : defs) m_editDefinitionList->addItem(def.trimmed());

    m_editDefinitionInput->clear();
    m_editDialog->show();
    m_editWordInput->setFocus();
}

void WordMemorizer::onSaveWordEdit() {
    QString newWord = m_editWordInput->text().trimmed();

    if (newWord.isEmpty()) {
        QMessageBox::warning(this, "保存失败", "单词不能为空");
        return;
    }

    QStringList definitions;
    for (int i = 0; i < m_editDefinitionList->count(); ++i)
        definitions << m_editDefinitionList->item(i)->text().trimmed();

    if (definitions.isEmpty()) {
        QMessageBox::warning(this, "保存失败", "至少需要一个释义");
        return;
    }

    QString newDefinition = definitions.join(", ");

    int newPriority = m_editPrioritySpin->value();

    std::optional<WordDatabaseError> err =
        m_db.updateWord(m_editingOriginalWord, newWord, newDefinition, newPriority);
    if (err) {
        QMessageBox::warning(this, "修改失败", err.value());
        return;
    }

    err = m_db.saveToFile();
    if (err) {
        QMessageBox::warning(this, "保存失败", err.value());
        return;
    }

    syncAndUpdateWordList();
    m_editDialog->hide();
}

void WordMemorizer::onCancelWordEdit() { m_editDialog->hide(); }

void WordMemorizer::setupStyle() {
    QFile styleFile(STYLE_FILE_PATH);

    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream styleStream(&styleFile);
        QString styleSheet = styleStream.readAll();
        setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        QMessageBox::critical(this, "错误", "无法加载样式表");
    }
}
