#ifndef WORDMEMORIZER_HPP
#define WORDMEMORIZER_HPP

#include <QComboBox>
#include <QCompleter>
#include <QElapsedTimer>
#include <QEvent>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSpinBox>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QVariant>

#include "worddatabase.hpp"

class PracticeResultDelegate : public QStyledItemDelegate {
public:
    explicit PracticeResultDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        QVariant isIncorrect = index.data(Qt::UserRole);
        if (isIncorrect.isValid() && isIncorrect.toBool()) {
            painter->fillRect(opt.rect, QColor(254, 240, 240));
            painter->setPen(QColor(245, 108, 108));
            painter->fillRect(opt.rect.left(), opt.rect.top(), 4, opt.rect.height(), QColor(245, 108, 108));
        } else {
            painter->fillRect(opt.rect, QColor(240, 249, 235));
            painter->setPen(QColor(103, 194, 58));
            painter->fillRect(opt.rect.left(), opt.rect.top(), 4, opt.rect.height(), QColor(103, 194, 58));
        }

        painter->drawText(opt.rect.adjusted(12, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, opt.text);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(size.height() + 8);
        return size;
    }
};

class WordMemorizer : public QMainWindow {
    Q_OBJECT

public:
    explicit WordMemorizer(QWidget *parent = nullptr);

    enum class PracticeMode {
        WordToDefinition = 0,
        DefinitionToWord,
    };

    enum class SortOrder {
        CreationTimeAsc = 0,
        CreationTimeDesc,
        ModifiedAtAsc,
        ModifiedAtDesc,
        PriorityAsc,
        PriorityDesc,
    };

private slots:
    void onAddWord();
    void onDeleteWord();
    void onStartPractice();
    void onPracticeSubmit();
    void onExitPractice();
    void onWordListContextMenuRequested(const QPoint &pos);
    void onEditWord();
    void onSaveWordEdit();
    void onCancelWordEdit();
    void onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onSortOrderChanged();
    void onSearchEditReturnPressed();

private:
    void setupUi();
    void setupStyle();
    void updateWordList();
    void startModeWordToDef();
    void startModeDefToWord();
    void endPractice();
    void updateTimerDisplay();
    void syncWordListDataCacheFromDb();
    void syncAndUpdateWordList();
    void loadSearchCompleterModel();
    void updateSearchCompleterModel(const QString &oldWord, const QString &newWord);

    WordDatabase m_db;

    // 单词录入
    QLineEdit *m_wordEdit;
    QLineEdit *m_definitionEdit;
    QSpinBox *m_prioritySpin;
    QListWidget *m_definitionListWidget;
    QPushButton *m_addDefinitionButton;
    QPushButton *m_removeDefinitionButton;
    QPushButton *m_addButton;
    QPushButton *m_deleteButton;

    // 单词列表
    bool m_isSearching;
    QPushButton *m_searchCancelButton;
    QLineEdit *m_searchEdit;
    QCompleter *m_searchCompleter;
    QStringListModel *m_searchModel;
    QVector<std::tuple<qsizetype, const WordEntry *>> m_wordListDataCache;
    QComboBox *m_wordListSortOrderCombo;
    SortOrder m_wordListSortOrder;
    QListWidget *m_wordListWidget;
    QMenu *m_wordListContextMenu;
    QAction *m_editWordAction;
    QAction *m_deleteWordAction;
    QDialog *m_editDialog;
    QLineEdit *m_editWordInput;
    QSpinBox *m_editPrioritySpin;
    QListWidget *m_editDefinitionList;
    QLineEdit *m_editDefinitionInput;
    QPushButton *m_editAddDefButton;
    QPushButton *m_editRemoveDefButton;
    QPushButton *m_editSaveButton;
    QPushButton *m_editCancelButton;
    QString m_editingOriginalWord;

    // 练习设置
    QPushButton *m_startPracticeButton;
    QComboBox *m_practiceModeCombo;
    QSpinBox *m_wordCountSpin;

    // 练习界面
    QLabel *m_questionLabel;
    QLineEdit *m_answerEdit;
    QPushButton *m_submitButton;
    QVector<qsizetype> m_practiceWordIndices;
    QLabel *m_practiceTimerLabel;
    QTimer *m_practiceTimer;
    QPushButton *m_exitPracticeButton;
    QLabel *m_practiceAccuracyLabel;
    QListWidget *m_practiceWordsListWidget;
    qint64 m_elapsedSeconds;
    int m_currentIndex;
    int m_correctCount;
    int m_totalQuestions;
    PracticeMode m_currentMode;
};

#endif  // WORDMEMORIZER_H
