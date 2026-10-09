#include "widgets/edit_word_dialog.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "widgets/definition_editor_widget.h"
#include "widgets/theme.h"

EditWordDialog::EditWordDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("编辑单词");
    setModal(true);
    resize(450, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // 单词基本信息表单
    QFormLayout *wordFormLayout = new QFormLayout;
    wordFormLayout->setSpacing(10);

    m_wordInput = new QLineEdit;
    m_wordInput->setObjectName("editWordInput");

    m_prioritySpin = new QSpinBox;
    m_prioritySpin->setObjectName("editPrioritySpin");
    m_prioritySpin->setRange(1, 20);
    m_prioritySpin->setValue(1);

    wordFormLayout->addRow("单词:", m_wordInput);
    wordFormLayout->addRow("优先级:", m_prioritySpin);
    mainLayout->addLayout(wordFormLayout);

    // 释义编辑区域（复用 DefinitionEditorWidget）
    QGroupBox *defGroup = new QGroupBox("释义");
    QVBoxLayout *defLayout = new QVBoxLayout(defGroup);

    m_definitionEditor = new DefinitionEditorWidget;
    m_definitionEditor->setPlaceholder("输入释义后点击添加");
    defLayout->addWidget(m_definitionEditor);

    mainLayout->addWidget(defGroup);

    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(10);

    m_saveButton = new QPushButton("保存");
    m_saveButton->setObjectName("editSaveButton");
    Theme::setVariant(m_saveButton, Theme::Variant::Success);

    m_cancelButton = new QPushButton("取消");
    m_cancelButton->setObjectName("editCancelButton");
    Theme::setVariant(m_cancelButton, Theme::Variant::Danger);

    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_saveButton, &QPushButton::clicked, this,
            &EditWordDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this,
            &EditWordDialog::onCancelClicked);
}

void EditWordDialog::loadWord(const QString &word,
                              const QStringList &definitions, int priority) {
    m_wordInput->setText(word);
    m_definitionEditor->setDefinitions(definitions);
    m_prioritySpin->setValue(priority);
    m_wordInput->setFocus();
    m_wordInput->selectAll();
}

void EditWordDialog::clearAndFocus() {
    m_wordInput->clear();
    m_definitionEditor->clear();
    m_prioritySpin->setValue(1);
    m_wordInput->setFocus();
}

void EditWordDialog::onSaveClicked() {
    QString word = m_wordInput->text().trimmed();
    QStringList definitions = m_definitionEditor->definitions();
    int priority = m_prioritySpin->value();

    emit saveRequested(word, definitions, priority);
}

void EditWordDialog::onCancelClicked() {
    emit cancelled();
    hide();
}
