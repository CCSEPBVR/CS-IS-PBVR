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
    int m_limit_min_time_step;
    int m_limit_max_time_step;
    void onValueChanged();
    void setLimitMinTimeStep(int min);
    void setLimitMaxTimeStep(int max);

};

#endif // TIMECONTROL_H
