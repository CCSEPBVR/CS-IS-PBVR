#ifndef TIMECONTROL_H
#define TIMECONTROL_H

#include <QWidget>

namespace Ui {
class TimeControl;
}

class TimeControl : public QWidget
{
    Q_OBJECT

public:
    explicit TimeControl(QWidget *parent = nullptr);
    ~TimeControl();

private:
    Ui::TimeControl *ui;
    int m_current_time_step;
    int m_next_time_step;
    int m_limit_min_time_step;
    int m_limit_max_time_step;
    bool m_is_loop;

    void setNextTimeStep(int nextTimeStep);
    void setLimitMinTimeStep(int min);
    void setLimitMaxTimeStep(int max);

    void onNextTimeStepChanged();
    void onLimitMinTimeStepChanged();
    void onLimitMaxTimeStepChanged();
    void onApplyButton();
    void toggleLoop();

public:
    int getCurrentTimeStep() { return m_current_time_step; }
    int getNextTimeStep() { return m_next_time_step; }
    int getLimitMinTimeStep() { return m_limit_min_time_step; }
    int getLimitMaxTimeStep() { return m_limit_max_time_step; }
    void updateTimeStepMinMax(int min,int max, bool isSingleObject);
    void setCurrentTimeStep(int current);
};

#endif // TIMECONTROL_H
