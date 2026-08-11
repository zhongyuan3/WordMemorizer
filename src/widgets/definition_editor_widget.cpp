#include "widgets/definition_editor_widget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

DefinitionEditorWidget::DefinitionEditorWidget(QWidget *parent)
    : QWidget(parent) {
    m_input = new QLineEdit;
    m_input->setObjectName("definitionInput");

    m_addButton = new QPushButton("添加释义");
    m_addButton->setObjectName("addDefinitionButton");

    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->setSpacing(8);
    inputLayout->addWidget(m_input);
    inputLayout->addWidget(m_addButton);

    m_list = new QListWidget;
    m_list->setObjectName("definitionList");

    m_removeButton = new QPushButton("删除选中的释义");
    m_removeButton->setObjectName("removeDefinitionButton");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_list);
    mainLayout->addWidget(m_removeButton);

    connect(m_addButton, &QPushButton::clicked, this,
            &DefinitionEditorWidget::onAddDefinition);
    connect(m_input, &QLineEdit::returnPressed, m_addButton,
            &QPushButton::click);
    connect(m_removeButton, &QPushButton::clicked, this,
            &DefinitionEditorWidget::onRemoveDefinition);
}

QStringList DefinitionEditorWidget::definitions() const {
    QStringList result;
    for (int i = 0; i < m_list->count(); ++i) {
        result << m_list->item(i)->text().trimmed();
    }
    return result;
}

void DefinitionEditorWidget::setDefinitions(const QStringList &defs) {
    m_list->clear();
    for (const QString &def : defs) {
        m_list->addItem(def.trimmed());
    }
}

void DefinitionEditorWidget::clear() {
    m_list->clear();
    m_input->clear();
}

void DefinitionEditorWidget::setPlaceholder(const QString &text) {
    m_input->setPlaceholderText(text);
}

void DefinitionEditorWidget::onAddDefinition() {
    QString input = m_input->text().trimmed();
    if (input.isEmpty()) return;

    // 支持空格分隔批量添加
    const QStringList defs = input.split(" ", Qt::SkipEmptyParts);
    m_list->addItems(defs);
    m_input->clear();
    m_input->setFocus();
}

void DefinitionEditorWidget::onRemoveDefinition() {
    QListWidgetItem *item = m_list->currentItem();
    if (item) {
        delete m_list->takeItem(m_list->row(item));
    }
}
