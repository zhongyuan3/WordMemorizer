#include "widgets/practice_record_widget.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QListView>
#include <QModelIndex>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <climits>

#include "widgets/theme.h"

PracticeRecordWidget::PracticeRecordWidget(QWidget *parent) : QWidget(parent) {
    // ---- 练习记录列表 ----
    m_recordList = new QListView;
    m_recordList->setObjectName("practiceRecordView");

    // ---- 练习配置 ----
    m_modeCombo = new QComboBox;
    m_modeCombo->setObjectName("practiceModeCombo");
    m_modeCombo->addItem("给出单词，填写释义");
    m_modeCombo->addItem("给出释义，填写单词");

    m_countSpin = new QSpinBox;
    m_countSpin->setObjectName("practiceCountSpin");
    m_countSpin->setRange(1, INT_MAX);
    m_countSpin->setValue(5);

    m_startButton = new QPushButton("开始练习");
    m_startButton->setObjectName("startPracticeButton");
    Theme::setVariant(m_startButton, Theme::Variant::Primary);

    QHBoxLayout *optionsLayout = new QHBoxLayout;
    optionsLayout->setSpacing(10);
    optionsLayout->addWidget(m_modeCombo);
    optionsLayout->addWidget(m_countSpin);
    optionsLayout->addWidget(m_startButton);

    // ---- 主布局 ----
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->addLayout(optionsLayout);
    mainLayout->addWidget(m_recordList);

    // ---- 信号连接 ----
    connect(m_startButton, &QPushButton::clicked, this,
            &PracticeRecordWidget::onStartClicked);
    connect(m_recordList, &QListView::doubleClicked, this,
            &PracticeRecordWidget::onRecordDoubleClicked);
}

void PracticeRecordWidget::setModel(QAbstractItemModel *model) {
    m_recordList->setModel(model);
}

void PracticeRecordWidget::onStartClicked() {
    emit startPracticeRequested(m_modeCombo->currentIndex(),
                                m_countSpin->value());
}

void PracticeRecordWidget::onRecordDoubleClicked(const QModelIndex &index) {
    if (!index.isValid()) return;

    // 通过 UserRole+1 映射获取实际记录行号
    // （对应 PracticeRecordModel::RecordIndexRole）
    QVariant mapped = index.data(Qt::UserRole + 1);
    int row = mapped.isValid() ? mapped.toInt() : index.row();
    emit recordDoubleClicked(row);
}
