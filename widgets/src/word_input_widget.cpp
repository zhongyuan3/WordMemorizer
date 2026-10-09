#include "widgets/word_input_widget.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

#include "widgets/definition_editor_widget.h"
#include "widgets/theme.h"

WordInputWidget::WordInputWidget(QWidget *parent) : QWidget(parent) {
    m_wordEdit = new QLineEdit;
    m_wordEdit->setObjectName("wordInputEdit");

    m_prioritySpin = new QSpinBox;
    m_prioritySpin->setObjectName("priorityInputSpin");
    m_prioritySpin->setRange(1, 20);
    m_prioritySpin->setValue(1);

    m_definitionEditor = new DefinitionEditorWidget;
    m_definitionEditor->setPlaceholder("输入释义后点击添加");

    m_addButton = new QPushButton("添加单词");
    m_addButton->setObjectName("addWordButton");
    Theme::setVariant(m_addButton, Theme::Variant::Success);

    m_deleteButton = new QPushButton("删除选中单词");
    m_deleteButton->setObjectName("deleteWordButton");
    Theme::setVariant(m_deleteButton, Theme::Variant::Danger);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(10);
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_deleteButton);

    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->setSpacing(10);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->addRow("单词", m_wordEdit);
    formLayout->addRow("优先级", m_prioritySpin);
    formLayout->addRow("释义", m_definitionEditor);
    formLayout->addRow(buttonLayout);

    connect(m_addButton, &QPushButton::clicked, this,
            &WordInputWidget::onAddClicked);
    connect(m_deleteButton, &QPushButton::clicked, this,
            &WordInputWidget::onDeleteClicked);
}

void WordInputWidget::onAddClicked() {
    QString word = m_wordEdit->text().trimmed();
    QStringList definitions = m_definitionEditor->definitions();
    int priority = m_prioritySpin->value();

    emit addWordRequested(word, definitions, priority);

    // 清空输入，准备录入下一个单词（保留优先级设置）
    m_wordEdit->clear();
    m_definitionEditor->clear();
    m_wordEdit->setFocus();
}

void WordInputWidget::onDeleteClicked() { emit deleteWordRequested(); }
