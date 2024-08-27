#ifndef TIMECONTROLLER_H
#define TIMECONTROLLER_H

#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidgetAction>
#include <QPushButton>
#include <QComboBox>>
#include <QTimer>

class MergePanel2;

/* A */
class TimeControllerA : public QToolBar
{
    Q_OBJECT

public:
    explicit TimeControllerA(QWidget *parent = nullptr);
    ~TimeControllerA();

    QSpinBox* getCurrentTimeStepLineEdit() const { return m_current_time_step_spin_box; }
    QSpinBox* getJumpTimeStepSpinBox() const { return m_jump_time_step_spin_box; }
    QSpinBox* getMinLimitTimeStepSpinBox() const { return m_min_limit_time_step_spin_box; };
    QSpinBox* getMaxLimitTimeStepSpinBox() const { return m_max_limit_time_step_spin_box; };
    QLabel* getTotalTimeStepRangeLabel() const { return m_total_time_step_range_label; }
    QSpinBox* getUpdateIntervalSpinBox() const { return m_update_interval_spin_box; };

private:
    QLabel *m_current_time_step_label;
    QSpinBox *m_current_time_step_spin_box;
    QLabel *m_jump_time_step_label;
    QSpinBox *m_jump_time_step_spin_box;
    QLabel *m_min_limit_time_step_label;
    QSpinBox *m_min_limit_time_step_spin_box;
    QLabel *m_max_limit_time_step_label;
    QSpinBox *m_max_limit_time_step_spin_box;
    QLabel *m_total_time_step_range_label;
    QLabel *m_update_interval_label;
    QSpinBox *m_update_interval_spin_box;
};

/* B */
class TimeControllerB : public QToolBar
{
    Q_OBJECT

public:
    explicit TimeControllerB(QWidget *parent = nullptr, TimeControllerA *time_controller_a = nullptr , MergePanel2* merge = nullptr);
    ~TimeControllerB();

    bool getLoopButtonEnabled() const;
    void updateMinMax( int min, int max, bool isSingleObject );
    void updateTimeStep();
//    void setMerge( MergePanel2* merge ){  m_merge = merge; }

    TimeControllerA* getTimeControllerA(){ return m_time_controller_a; }

private:
    TimeControllerA *m_time_controller_a;

    QPushButton* createButton(const QString &iconPath, const QSize &iconSize, const QSize &buttonSize, QWidget *parent);
    void disableButtons();
    void enableButtons();

    QPushButton *m_first_time_step_push_button;
    QPushButton *m_previous_time_step_push_button;
    QPushButton *m_reverse_push_button;
    QPushButton *m_play_push_button;
    QPushButton *m_next_time_step_push_button;
    QPushButton *m_last_time_step_push_button;
    QPushButton *m_jump_push_button;
    QPushButton *m_loop_push_button;

    QTimer m_timer;

    MergePanel2* m_merge;

private slots:
    void onFirst();
    void onPrevious();
    void onReverse();
    void onPlay();
    void onNext();
    void onLast();
    void onJump();
    void onLoop();

    void onTimerStart();

    void onMinLimit();
    void onMaxLimit();
};

/* TotalParticles */
class TotalParticles :public QToolBar
{
    Q_OBJECT

public:
    explicit TotalParticles( QWidget *parent = nullptr );
    ~TotalParticles();

    void setTotalParticles( int totalParticles );

private:
    QLabel* m_total_particles_label;
    QLineEdit* m_total_particles_line_edit;
};

/* ColorMapBarSelector */
class ColorMapBarSelector :public QToolBar
{
    Q_OBJECT

public:
    explicit ColorMapBarSelector( QWidget *parent = nullptr );
    ~ColorMapBarSelector();

private:
    QLabel* m_color_map_bar_selector_label;
    QComboBox* m_color_map_bar_selector_combo_box;
};

#endif // TIMECONTROLLER_H
