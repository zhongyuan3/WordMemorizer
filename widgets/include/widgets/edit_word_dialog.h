#ifndef EDIT_WORD_DIALOG_H
#define EDIT_WORD_DIALOG_H

#include <QDialog>

class QLineEdit;
class QSpinBox;
class QPushButton;
class DefinitionEditorWidget;

/**
 * @brief 编辑单词对话框
 *
 * 替代主窗口中原有的内联编辑对话框（m_editDialog）。复用
 * DefinitionEditorWidget 组件，统一释义编辑逻辑，消除重复代码。
 */
class EditWordDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditWordDialog(QWidget *parent = nullptr);

    /// 使用待编辑单词数据填充对话框
    void loadWord(const QString &word, const QStringList &definitions,
                  int priority);

    /// 清空所有输入并聚焦到单词输入框
    void clearAndFocus();

private slots:
    void onSaveClicked();
    void onCancelClicked();

signals:
    void saveRequested(QString word, QStringList definitions, int priority);
    void cancelled();

private:
    QLineEdit *m_wordInput;
    QSpinBox *m_prioritySpin;
    DefinitionEditorWidget *m_definitionEditor;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};

#endif  // EDIT_WORD_DIALOG_H
