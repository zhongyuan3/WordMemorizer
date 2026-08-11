#ifndef PRACTICE_RECORD_DETAIL_H
#define PRACTICE_RECORD_DETAIL_H

#include <QCloseEvent>
#include <QWidget>

#include "data/models.h"

class QLabel;
class QListWidget;

/**
 * @brief 练习记录详情窗口
 *
 * 重构要点：替代原 WordMemorizer 中的 PracticeRecordDisplayer（QMainWindow），
 * 改为继承 QWidget，降低耦合。展示单次练习的时间、时长、正确率、模式
 * 以及错题列表。关闭时隐藏而非销毁，便于复用。
 */
class PracticeRecordDetail : public QWidget {
    Q_OBJECT

public:
    explicit PracticeRecordDetail(QWidget *parent = nullptr);

    /// 展示指定练习记录的详情
    void displayRecord(const PracticeRecord &record);

protected:
    void closeEvent(QCloseEvent *event) override {
        event->ignore();
        hide();
    }

private:
    QLabel *m_timeLabel;
    QLabel *m_durationLabel;
    QLabel *m_rateLabel;
    QLabel *m_modeLabel;
    QListWidget *m_wrongWordsList;
};

#endif  // PRACTICE_RECORD_DETAIL_H
