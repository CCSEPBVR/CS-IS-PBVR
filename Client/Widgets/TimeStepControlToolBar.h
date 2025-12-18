#ifndef TIMESTEPCONTROLTOOLBAR_H
#define TIMESTEPCONTROLTOOLBAR_H

#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include "WebSocketPair.h"

#include "../../Shared/JsonKeys.h"

class TimeStepControlToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit TimeStepControlToolBar( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~TimeStepControlToolBar();

public slots:
    void onUpdateServerState( bool serverState );     // true:接続中
    void onOperatorStateUpdate( bool operatorState ); // true:権限あり
    void onReset();
    void updateTotalTimeStepRange( int min, int max, bool isSingleObject );
    void onReceiveTimeStepControlParameter( const QJsonObject& timeStepControlParameter );
    void onDataRequestCompleted( int requestTimeStep );

    void onFirst();
    void onPrevious();
    void onReverse( bool isChecked );
    void onPlay( bool isChecked );
    void onNext();
    void onLast();
    void onKeepLast( bool isChecked );
    void onJump();
    void onLoop( bool isChecked );
    // FIXME:KPI
    void onLoadParameter( const QString& filePath );
    void onSaveParameter( const QString& filePath );

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
    void onUpdateNext( int next );
    void onUpdateMinLimit( int minLimit );
    void onUpdateMaxLimit( int maxLimit );
    void onUpdateInterval();
    void onUpdateTimeStep();
};

#endif // TIMESTEPCONTROLTOOLBAR_H
