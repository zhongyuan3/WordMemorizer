#ifndef DEFINITION_EDITOR_WIDGET_H
#define DEFINITION_EDITOR_WIDGET_H

#include <QWidget>

class QLineEdit;
class QListWidget;
class QPushButton;

/**
 * @brief 释义列表编辑器（可复用组件）
 *
 * 重构要点：旧版在"添加单词"和"编辑单词对话框"中各写了一份几乎相同的
 * 释义添加/删除逻辑。此组件将两者统一，消除代码重复。
 */
class DefinitionEditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit DefinitionEditorWidget(QWidget *parent = nullptr);

    /// 获取当前所有释义
    QStringList definitions() const;

    /// 设置释义列表（替换现有内容）
    void setDefinitions(const QStringList &defs);

    /// 清空所有释义
    void clear();

    /// 设置输入框的 placeholder
    void setPlaceholder(const QString &text);

private slots:
    void onAddDefinition();
    void onRemoveDefinition();

private:
    QLineEdit *m_input;
    QListWidget *m_list;
    QPushButton *m_addButton;
    QPushButton *m_removeButton;
};

#endif  // DEFINITION_EDITOR_WIDGET_H
