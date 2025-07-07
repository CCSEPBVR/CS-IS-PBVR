#ifndef TIMESTEPCONTROLTOOLBAR_H
#define TIMESTEPCONTROLTOOLBAR_H

#include <QToolBar>
#include <QLabel>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QTimer>

class TimeStepControlToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit TimeStepControlToolBar( QWidget *parent = nullptr );
    ~TimeStepControlToolBar();

private:
    void initialize();

private:
    QLabel *m_current_time_step_label;
    QLabel *m_current_time_step_display;

    QLabel *m_jump_time_step_label;
    QSpinBox *m_jump_time_step_spin_box;

    QLabel *m_min_limit_time_step_label;
    QSpinBox *m_min_limit_time_step_spin_box;

    QLabel *m_max_limit_time_step_label;
    QSpinBox *m_max_limit_time_step_spin_box;

    QLabel *m_total_time_step_range_label;

    QLabel *m_update_interval_label;
    QSpinBox *m_update_interval_spin_box;

    QTimer m_timer;
    bool m_is_reverse_mode = false;
    bool m_is_last_mode = false;
    bool m_is_merging = false;

signals:
    void requestMerge( int requestTimeStep );
    void doneMerge();

public slots:
    void updateTotalTimeStepRange( int min, int max, bool isSingleObject );

    void fisrtTimeStep();
    void previousTimeStep();
    void reverseTimeStep( bool );
    void playTimeStep( bool );
    void nextTimeStep();
    void lastTimeStep();
    void keepLastTimeStep( bool );
    void jumpTimeStep();
    void loopMode( bool );
    void mergeFinish( int );
    void noItems();
    void updateInSituObjectMinMaxTimeStep( int , int );

private slots:
    void updateMinLimit( int );
    void updateMaxLimit( int );
    void updateInternal();
    void updateTimeStep();
    void decrementTimeStep();
    void incrementTimeStep();
    void keepLast();
};

#endif // TIMESTEPCONTROLTOOLBAR_H
