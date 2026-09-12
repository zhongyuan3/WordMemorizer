#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include "core/practice_session.h"
#include "data/word_database.h"

class QGroupBox;
class QLabel;
class QSortFilterProxyModel;
class QTimer;
class WordListModel;
class PracticeRecordModel;
class WordInputWidget;
class WordListWidget;
class EditWordDialog;
class PracticeWidget;
class PracticeRecordWidget;
class PracticeRecordDetail;

/**
 * @brief 主窗口（组合根）
 *
 * 重构要点：
 * - 旧版 WordMemorizer 是一个 ~1400 行的 God Object，同时负责 UI 创建、
 *   业务逻辑、数据管理。重构后 MainWindow 仅作为组合根，负责创建各组件、
 *   连接信号/槽，不包含任何业务逻辑或 UI 布局细节。
 * - 业务逻辑下沉到 PracticeSession、WordDatabase 等独立类中。
 * - UI 布局下沉到各 Widget 子类中。
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // ---- 单词管理 ----
    void onAddWordRequested(const QString &word, const QStringList &definitions,
                            int priority);
    void onDeleteWordRequested();
    void onEditWordRequested(int sourceRow);
    void onViewDefinitionRequested(int sourceRow);
    void onEditSaveRequested(const QString &word,
                             const QStringList &definitions, int priority);

    // ---- 练习 ----
    void onStartPracticeRequested(int mode, int count);
    void onPracticeExitRequested();
    void onPracticeSessionStarted();
    void onPracticeSessionEnded();

    // ---- 练习记录 ----
    void onPracticeRecordDoubleClicked(int row);

    // ---- 定时器 ----
    void onPracticeTimerTimeout();

private:
    void initializeDatabase();
    void setupUi();
    void setupStyle();
    void setupConnections();
    void saveDatabase();
    void updateStatusBar();

    // 数据层
    WordDatabase m_db;

    // Model 层
    WordListModel *m_wordModel = nullptr;
    PracticeRecordModel *m_recordModel = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;

    // 业务逻辑层
    PracticeSession *m_session = nullptr;
    QTimer *m_practiceTimer = nullptr;

    // UI 组件
    QGroupBox *m_wordsGroup = nullptr;
    QGroupBox *m_recordGroup = nullptr;
    WordInputWidget *m_inputWidget = nullptr;
    WordListWidget *m_listWidget = nullptr;
    EditWordDialog *m_editDialog = nullptr;
    PracticeWidget *m_practiceWidget = nullptr;
    PracticeRecordWidget *m_recordWidget = nullptr;
    PracticeRecordDetail *m_recordDetail = nullptr;
    QLabel *m_statusLabel = nullptr;

    // 当前编辑的原始单词（用于 updateWord 查找）
    QString m_editingOriginalWord;
};

#endif  // MAIN_WINDOW_H
