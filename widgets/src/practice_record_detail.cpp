#include "widgets/practice_record_detail.h"

#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

PracticeRecordDetail::PracticeRecordDetail(QWidget *parent) : QWidget(parent) {
    setWindowTitle("练习记录详情");
    resize(400, 500);

    m_timeLabel = new QLabel;
    m_durationLabel = new QLabel;
    m_rateLabel = new QLabel;
    m_modeLabel = new QLabel;

    m_wrongWordsList = new QListWidget;

    QFormLayout *infoLayout = new QFormLayout;
    infoLayout->addRow("练习时间：", m_timeLabel);
    infoLayout->addRow("时长：", m_durationLabel);
    infoLayout->addRow("正确率：", m_rateLabel);
    infoLayout->addRow("模式：", m_modeLabel);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->addLayout(infoLayout);
    mainLayout->addWidget(new QLabel("错题列表："));
    mainLayout->addWidget(m_wrongWordsList);
}

void PracticeRecordDetail::displayRecord(const PracticeRecord &record) {
    m_timeLabel->setText(record.practiceTime.toString("yyyy-MM-dd hh:mm:ss"));
    m_durationLabel->setText(record.duration.toString("hh:mm:ss"));
    m_rateLabel->setText(QString::number(record.correctRate, 'f', 2) + "%");

    QString modeText = (record.mode == PracticeMode::WordToDefinition)
                           ? "给出单词，填写释义"
                           : "给出释义，填写单词";
    m_modeLabel->setText(modeText);

    m_wrongWordsList->clear();
    for (const PracticeEntry &entry : record.wrongWords) {
        QString itemText = QString("%1 - %2 (你的答案: %3)")
                               .arg(entry.word)
                               .arg(entry.definition)
                               .arg(entry.userAnswer);
        m_wrongWordsList->addItem(itemText);
    }
}
