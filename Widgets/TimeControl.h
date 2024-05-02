#ifndef TIMECONTROL_H
#define TIMECONTROL_H

#include <QWidget>
#include <QTimer>

class MergePanel;

namespace Ui {
class TimeControl;
}

class TimeControl : public QWidget
{
    Q_OBJECT

public:
    explicit TimeControl(QWidget *parent = nullptr);
    ~TimeControl();
    void setMerge( MergePanel* merge ){  m_merge = merge; }
    void incrementSpinBox();

private:
    Ui::TimeControl *ui;
    int m_current_time_step;
    int m_next_time_step;
    int m_limit_min_time_step;
    int m_limit_max_time_step;
    bool m_is_loop;
    QTimer m_timer;
    MergePanel* m_merge;

    void setNextTimeStep(int nextTimeStep);
    void setLimitMinTimeStep(int min);
    void setLimitMaxTimeStep(int max);

    void onNextTimeStepChanged();
    void onLimitMinTimeStepChanged();
    void onLimitMaxTimeStepChanged();
    void onPlayToggleButtonClicked();
    void onTimerStart();
    void onStepIntervalChanged();

public:
    int getCurrentTimeStep() { return m_current_time_step; }
    int getNextTimeStep() { return m_next_time_step; }
    int getLimitMinTimeStep() { return m_limit_min_time_step; }
    int getLimitMaxTimeStep() { return m_limit_max_time_step; }
    void updateTimeStepMinMax(int min,int max, bool isSingleObject);
    void setCurrentTimeStep(int current);    
};

#endif // TIMECONTROL_H
