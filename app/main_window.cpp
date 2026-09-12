#include "main_window.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

#include "models/practice_record_model.h"
#include "models/word_list_model.h"
#include "widgets/edit_word_dialog.h"
#include "widgets/practice_record_detail.h"
#include "widgets/practice_record_widget.h"
#include "widgets/practice_widget.h"
#include "widgets/word_input_widget.h"
#include "widgets/word_list_widget.h"

static constexpr const char *STYLE_FILE_PATH = ":/resources/styles/main.qss";
static constexpr const char *DATA_FILE_NAME = "words.json";
static constexpr const char *PRACTICE_RECORDS_FILE_NAME =
    "practice_records.json";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    initializeDatabase();

    m_wordModel = new WordListModel(&m_db, this);
    m_recordModel = new PracticeRecordModel(&m_db, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_wordModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(0);

    m_session = new PracticeSession(&m_db, this);
    m_practiceTimer = new QTimer(this);
    m_practiceTimer->setInterval(1000);

    setupUi();
    setupStyle();
    setupConnections();

    m_wordModel->refreshAll();
    m_recordModel->refreshAll();

    setWindowTitle("单词记忆助手");
    setWindowIcon(QIcon(":/resources/icons/logo.png"));
    setMinimumSize(900, 600);
    updateStatusBar();
}

MainWindow::~MainWindow() { saveDatabase(); }

// ============================================================
// 初始化
// ============================================================

void MainWindow::initializeDatabase() {
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

    m_db.setWordsFilename(writeDir.absoluteFilePath(DATA_FILE_NAME));
    m_db.setPracticeRecordsFilename(
        writeDir.absoluteFilePath(PRACTICE_RECORDS_FILE_NAME));

    auto err = m_db.loadFromFile();
    if (err) {
        QMessageBox::critical(this, "错误", err.value().message());
    }
}

void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // ---- 单词区域（录入 + 列表） ----
    m_inputWidget = new WordInputWidget;
    m_listWidget = new WordListWidget;
    m_listWidget->setModel(m_wordModel, m_proxyModel);

    m_wordsGroup = new QGroupBox;
    m_wordsGroup->setObjectName("wordsGroup");
    m_wordsGroup->setTitle("单词管理");
    QHBoxLayout *wordsLayout = new QHBoxLayout(m_wordsGroup);
    wordsLayout->setSpacing(10);
    wordsLayout->setContentsMargins(12, 20, 12, 12);
    wordsLayout->addWidget(m_inputWidget, 2);
    wordsLayout->addWidget(m_listWidget, 3);

    // ---- 练习记录区域 ----
    m_recordWidget = new PracticeRecordWidget;
    m_recordWidget->setModel(m_recordModel);

    m_recordGroup = new QGroupBox;
    m_recordGroup->setObjectName("practiceRecordGroup");
    m_recordGroup->setTitle("练习记录");
    QVBoxLayout *recordLayout = new QVBoxLayout(m_recordGroup);
    recordLayout->setSpacing(10);
    recordLayout->setContentsMargins(12, 20, 12, 12);
    recordLayout->addWidget(m_recordWidget);

    // ---- 练习区域 ----
    m_practiceWidget = new PracticeWidget;
    m_practiceWidget->setSession(m_session);
    m_practiceWidget->hide();

    // ---- 编辑对话框 ----
    m_editDialog = new EditWordDialog(this);

    // ---- 练习记录详情窗口 ----
    m_recordDetail = new PracticeRecordDetail(this);
    m_recordDetail->hide();

    // ---- 主布局 ----
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setSpacing(12);
    topLayout->addWidget(m_wordsGroup, 3);
    topLayout->addWidget(m_recordGroup, 2);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_practiceWidget);

    // ---- 状态栏 ----
    m_statusLabel = new QLabel;
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::setupStyle() {
    QFile styleFile(STYLE_FILE_PATH);
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString sheet = stream.readAll();
        qApp->setStyleSheet(sheet);
        styleFile.close();
    } else {
        QMessageBox::critical(this, "错误", "无法加载样式表");
    }
}

void MainWindow::setupConnections() {
    // 单词录入
    connect(m_inputWidget, &WordInputWidget::addWordRequested, this,
            &MainWindow::onAddWordRequested);
    connect(m_inputWidget, &WordInputWidget::deleteWordRequested, this,
            &MainWindow::onDeleteWordRequested);

    // 单词列表
    connect(m_listWidget, &WordListWidget::editRequested, this,
            &MainWindow::onEditWordRequested);
    connect(m_listWidget, &WordListWidget::deleteRequested, this,
            &MainWindow::onDeleteWordRequested);
    connect(m_listWidget, &WordListWidget::viewDefinitionRequested, this,
            &MainWindow::onViewDefinitionRequested);

    // 编辑对话框
    connect(m_editDialog, &EditWordDialog::saveRequested, this,
            &MainWindow::onEditSaveRequested);
    connect(m_editDialog, &EditWordDialog::cancelled, m_editDialog,
            &QDialog::hide);

    // 练习
    connect(m_recordWidget, &PracticeRecordWidget::startPracticeRequested, this,
            &MainWindow::onStartPracticeRequested);
    connect(m_practiceWidget, &PracticeWidget::exitRequested, this,
            &MainWindow::onPracticeExitRequested);
    connect(m_practiceTimer, &QTimer::timeout, this,
            &MainWindow::onPracticeTimerTimeout);
    connect(m_session, &PracticeSession::sessionStarted, this,
            &MainWindow::onPracticeSessionStarted);
    connect(m_session, &PracticeSession::sessionEnded, this,
            &MainWindow::onPracticeSessionEnded);
    connect(m_session, &PracticeSession::error, this,
            [this](const QString &msg) {
                QMessageBox::warning(this, "练习失败", msg, QMessageBox::Ok);
            });

    // 练习记录
    connect(m_recordWidget, &PracticeRecordWidget::recordDoubleClicked, this,
            &MainWindow::onPracticeRecordDoubleClicked);

    // 退出时保存
    connect(qApp, &QApplication::aboutToQuit, this,
            [this]() { saveDatabase(); });
}

// ============================================================
// 单词管理
// ============================================================

void MainWindow::onAddWordRequested(const QString &word,
                                    const QStringList &definitions,
                                    int priority) {
    if (word.isEmpty()) {
        QMessageBox::warning(this, "添加失败", "单词不能为空", QMessageBox::Ok);
        return;
    }
    if (definitions.isEmpty()) {
        QMessageBox::warning(this, "添加失败", "至少添加一个释义",
                             QMessageBox::Ok);
        return;
    }

    auto err = m_db.addWord(word, definitions, priority);
    if (err) {
        QMessageBox::warning(this, "添加失败", err.value().message(),
                             QMessageBox::Ok);
        return;
    }

    saveDatabase();
    m_wordModel->refreshAll();
    updateStatusBar();
}

void MainWindow::onDeleteWordRequested() {
    // 从 WordListWidget 获取当前选中项的源行号
    // 支持两种触发来源：inputWidget 的删除按钮 和 listWidget 的右键菜单
    int sourceRow = m_listWidget->currentSourceRow();
    if (sourceRow < 0 || sourceRow >= m_db.wordCount()) {
        QMessageBox::warning(this, "删除失败", "请先选择要删除的单词",
                             QMessageBox::Ok);
        return;
    }

    const WordEntry &entry = m_db.words()[sourceRow];

    auto reply = QMessageBox::question(
        this, "确认删除", QString("确认删除单词 \"%1\" 吗？").arg(entry.word),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    auto err = m_db.removeWord(entry.word);
    if (err) {
        QMessageBox::warning(this, "删除失败", err.value().message(),
                             QMessageBox::Ok);
        return;
    }

    saveDatabase();
    m_wordModel->refreshAll();
    updateStatusBar();
}

void MainWindow::onEditWordRequested(int sourceRow) {
    if (sourceRow < 0 || sourceRow >= m_db.wordCount()) {
        QMessageBox::warning(this, "编辑失败", "无效的单词索引",
                             QMessageBox::Ok);
        return;
    }

    const WordEntry &entry = m_db.words()[sourceRow];
    m_editingOriginalWord = entry.word;
    m_editDialog->loadWord(entry.word, entry.definitions, entry.priority);
    m_editDialog->show();
}

void MainWindow::onEditSaveRequested(const QString &word,
                                     const QStringList &definitions,
                                     int priority) {
    if (word.isEmpty()) {
        QMessageBox::warning(this, "修改失败", "单词不能为空");
        return;
    }
    if (definitions.isEmpty()) {
        QMessageBox::warning(this, "修改失败", "至少添加一个释义");
        return;
    }

    auto err =
        m_db.updateWord(m_editingOriginalWord, word, definitions, priority);
    if (err) {
        QMessageBox::warning(this, "修改失败", err.value().message());
        return;
    }

    saveDatabase();
    m_wordModel->refreshAll();
    updateStatusBar();
    m_editDialog->hide();
}

void MainWindow::onViewDefinitionRequested(int sourceRow) {
    if (sourceRow < 0 || sourceRow >= m_db.wordCount()) {
        QMessageBox::warning(this, "查看失败", "无效的单词索引",
                             QMessageBox::Ok);
        return;
    }

    const WordEntry &entry = m_db.words()[sourceRow];

    QString info = QString("单词：%1\n").arg(entry.word);
    info += QString("优先级：%1\n\n").arg(entry.priority);
    info += "释义：";
    for (const QString &def : entry.definitions) {
        info += QString("\n  \xe2\x80\xa2 %1").arg(def);
    }

    if (entry.totalAttempts > 0) {
        double errorRate =
            100.0 * entry.totalErrors / entry.totalAttempts;
        info += QString("\n\n练习统计：");
        info += QString("\n  总答题次数：%1").arg(entry.totalAttempts);
        info += QString("\n  总错误次数：%1").arg(entry.totalErrors);
        info += QString("\n  错误率：%1%")
                    .arg(QString::number(errorRate, 'f', 1));
    }

    QMessageBox::information(this, "查看释义", info, QMessageBox::Ok);
}

// ============================================================
// 练习
// ============================================================

void MainWindow::onStartPracticeRequested(int mode, int count) {
    PracticeMode practiceMode = static_cast<PracticeMode>(mode);
    if (!m_session->start(practiceMode, count)) {
        return;  // error 已通过信号处理
    }
}

void MainWindow::onPracticeSessionStarted() {
    m_practiceTimer->start();
    m_wordsGroup->hide();
    m_recordGroup->hide();
    m_practiceWidget->show();
    m_practiceWidget->reset();
}

void MainWindow::onPracticeSessionEnded() {
    m_practiceTimer->stop();
    m_recordModel->refreshAll();
}

void MainWindow::onPracticeExitRequested() {
    m_session->abort();
    m_practiceTimer->stop();
    m_practiceWidget->hide();
    m_wordsGroup->show();
    m_recordGroup->show();
    saveDatabase();
}

void MainWindow::onPracticeTimerTimeout() { m_session->tick(); }

// ============================================================
// 练习记录
// ============================================================

void MainWindow::onPracticeRecordDoubleClicked(int row) {
    if (row < 0 || row >= m_db.practiceRecordCount()) return;
    m_recordDetail->displayRecord(m_db.practiceRecords()[row]);
    m_recordDetail->show();
    m_recordDetail->raise();
    m_recordDetail->activateWindow();
}

// ============================================================
// 持久化
// ============================================================

void MainWindow::saveDatabase() {
    auto err = m_db.saveToFile();
    if (err) {
        QMessageBox::critical(this, "错误", err.value().message());
    }
}

void MainWindow::updateStatusBar() {
    m_statusLabel->setText(QString("共 %1 个单词").arg(m_db.wordCount()));
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveDatabase();
    event->accept();
}
