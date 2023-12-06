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
    int m_future_time_step = 0;
    int m_limit_min_time_step = INT_MAX;
    int m_limit_max_time_step = INT_MIN;
    bool m_is_loop;

    void setCurrentTimeStep(int current);
    void setFutureTimeStep(int future);
    void setLimitMinTimeStep(int min);
    void setLimitMaxTimeStep(int max);

    void onFutureTimeStepChanged();
    void onApplyButton();
    void toggleLoop();

public:
    int getCurrentTimeStep() { return m_current_time_step; }
    int getFutureTimeStep() { return m_future_time_step; }
    int getLimitMinTimeStep() { return m_limit_min_time_step; }
    int getLimitMaxTimeStep() { return m_limit_max_time_step; }
    void updateTimeStepMinMax(int min,int max);
};

#endif // TIMECONTROL_H
