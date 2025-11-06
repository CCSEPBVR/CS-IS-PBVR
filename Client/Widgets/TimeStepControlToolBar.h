#ifndef TIMESTEPCONTROLTOOLBAR_H
#define TIMESTEPCONTROLTOOLBAR_H

#include <QToolBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimer>

class TimeStepControlToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit TimeStepControlToolBar( QWidget *parent = nullptr );
    ~TimeStepControlToolBar();

public slots:
    void updateTotalTimeStepRange( int min, int max, bool isSingleObject );

    void first();
    void previous();
    void reverse( bool isChecked );
    void play( bool isChecked );
    void next();
    void last();
    void keepLast( bool isChecked );
    void jump();
    void loop( bool isChecked );

    void doneTimeControlToolBar( int requestTimeStep );

    // void noItems();

    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

signals:
    void requestMerge( int requestTimeStep );
    void done();

private:
    QLabel* m_current_time_step_label;
    QLineEdit* m_current_time_step_line_edit;

    QLabel* m_next_time_step_label;
    QSpinBox* m_next_time_step_spin_box;

    QLabel* m_min_limit_time_step_label;
    QSpinBox* m_min_limit_time_step_spin_box;

    QLabel* m_max_limit_time_step_label;
    QSpinBox* m_max_limit_time_step_spin_box;

    QLabel* m_total_time_step_range_label;

    QLabel* m_update_interval_label;
    QSpinBox* m_update_interval_spin_box;

    QTimer m_timer;
    bool m_is_reverse_mode  = false;
    bool m_is_last_mode     = false;
    bool m_is_merging       = false;

    void initialize();
    void decrementTimeStep();
    void incrementTimeStep();
    void keepLastTimeStep();

private slots:
    void updateMinLimit( int minLimit );
    void updateMaxLimit( int maxLimit );
    void updateInterval();
    void updateTimeStep();
};

#endif // TIMESTEPCONTROLTOOLBAR_H
