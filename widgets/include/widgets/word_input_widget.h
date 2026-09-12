#ifndef WORD_INPUT_WIDGET_H
#define WORD_INPUT_WIDGET_H

#include <QWidget>

class QLineEdit;
class QSpinBox;
class QPushButton;
class DefinitionEditorWidget;

/**
 * @brief 单词录入表单 Widget
 *
 * 将主窗口中原有的"单词录入"区域（单词输入、优先级、释义编辑、添加/删除按钮）
 * 抽取为独立组件。通过信号通知外部进行实际的数据库操作，实现 UI 与数据的解耦。
 */
class WordInputWidget : public QWidget {
    Q_OBJECT

public:
    explicit WordInputWidget(QWidget *parent = nullptr);

private slots:
    void onAddClicked();
    void onDeleteClicked();

signals:
    void addWordRequested(QString word, QStringList definitions, int priority);
    void deleteWordRequested();

private:
    QLineEdit *m_wordEdit;
    QSpinBox *m_prioritySpin;
    DefinitionEditorWidget *m_definitionEditor;
    QPushButton *m_addButton;
    QPushButton *m_deleteButton;
};

#endif  // WORD_INPUT_WIDGET_H
