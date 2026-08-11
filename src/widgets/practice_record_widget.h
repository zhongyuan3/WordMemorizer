#ifndef PRACTICE_RECORD_WIDGET_H
#define PRACTICE_RECORD_WIDGET_H

#include <QWidget>

class QAbstractItemModel;
class QComboBox;
class QListView;
class QSpinBox;
class QPushButton;

/**
 * @brief 练习记录列表 Widget
 *
 * 重构要点：将练习记录列表和练习启动配置从 WordMemorizer God Object
 * 中抽离为独立组件。通过 setModel() 接受任意 QAbstractItemModel，
 * 通过信号通知外部启动练习或查看记录详情。
 */
class PracticeRecordWidget : public QWidget {
    Q_OBJECT

public:
    explicit PracticeRecordWidget(QWidget *parent = nullptr);

    /// 设置练习记录列表的数据模型
    void setModel(QAbstractItemModel *model);

signals:
    /// 用户点击"开始练习"时发出，携带选定的模式和题目数量
    void startPracticeRequested(int mode, int count);

    /// 用户双击练习记录时发出，携带映射后的记录行号
    void recordDoubleClicked(int row);

private slots:
    void onStartClicked();
    void onRecordDoubleClicked(const QModelIndex &index);

private:
    QListView *m_recordList;
    QComboBox *m_modeCombo;
    QSpinBox *m_countSpin;
    QPushButton *m_startButton;
};

#endif  // PRACTICE_RECORD_WIDGET_H
