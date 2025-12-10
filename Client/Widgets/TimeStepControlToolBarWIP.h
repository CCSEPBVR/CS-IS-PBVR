#ifndef TIMESTEPCONTROLTOOLBARWIP_H
#define TIMESTEPCONTROLTOOLBARWIP_H

#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include "WebSocketPair.h"

class TimeStepControlToolBarWIP : public QToolBar
{
    Q_OBJECT
public:
    TimeStepControlToolBarWIP( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~TimeStepControlToolBarWIP();

public slots:
    void updateTotalTimeStepRange( int min, int max, bool isSingleObject );
    void onOperatorStateUpdate( bool operatorState );
    void onReceiveTimeStepControlParameter( const QJsonObject& timeStepControlParameter );
    void onDataRequestCompleted( int requestTimeStep );

    void first();
    void previous();
    void reverse( bool isChecked );
    void play( bool isChecked );
    void next();
    void last();
    void keepLast( bool isChecked );
    void jump();
    void loop( bool isChecked );

    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

signals:
    void requestDataAt( const int requestTimeStep );
    void dataRequestCompleted();

private:
    WebSocketPair* m_web_sockets = nullptr;
    bool m_is_operator;

    QLabel* m_current_time_step_label           = nullptr;
    QLineEdit* m_current_time_step_line_edit    = nullptr;

    QLabel* m_next_time_step_label              = nullptr;
    QSpinBox* m_next_time_step_spin_box         = nullptr;

    QLabel* m_min_limit_time_step_label         = nullptr;
    QSpinBox* m_min_limit_time_step_spin_box    = nullptr;

    QLabel* m_max_limit_time_step_label         = nullptr;
    QSpinBox* m_max_limit_time_step_spin_box    = nullptr;

    QLabel* m_total_time_step_range_label       = nullptr;

    QLabel* m_update_interval_label             = nullptr;
    QSpinBox* m_update_interval_spin_box        = nullptr;

    QTimer m_timer;
    bool m_is_reverse_mode  = false;
    bool m_is_last_mode     = false;
    bool m_is_merging       = false;

    void decrementTimeStep();
    void incrementTimeStep();
    void keepLastTimeStep();

private slots:
    void updateNext( int next );
    void updateMinLimit( int minLimit );
    void updateMaxLimit( int maxLimit );
    void updateInterval();
    void updateTimeStep();
};

#endif // TIMESTEPCONTROLTOOLBARWIP_H
