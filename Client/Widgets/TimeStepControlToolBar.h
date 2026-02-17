#ifndef TIMESTEPCONTROLTOOLBAR_H
#define TIMESTEPCONTROLTOOLBAR_H

#include <QToolBar>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimer>

#include "WebSocketPair.h"

#include "../../Shared/JsonKeys.h"

class TimeStepControlToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit TimeStepControlToolBar( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~TimeStepControlToolBar();
    void reset();

signals:
    void requestDataAt( const int requestTimeStep );
    void dataRequestCompleted( const int requestTimeStep );

public slots:
    void onOperatorStateUpdate( const bool operatorState ); // true: 権限有り, false: 権限無し

    // NOTE:テキストソケット用
    void onReceiveTimeStepControlParameter( const QJsonObject& timeStepControlParameter );

    void onUpdateTotalTimeStepRange( const int min, const int max, const bool isSingleObject );
    void onDataRequestCompleted( const int requestTimeStep );

    void onFirst();
    void onPrevious();
    void onReverse( bool isChecked );
    void onPlay( bool isChecked );
    void onNext();
    void onLast();
    void onKeepLast( bool isChecked );
    void onJump();
    void onLoop( bool isChecked );

    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    QLabel* m_current_time_step_label        = nullptr;
    QLineEdit* m_current_time_step_line_edit = nullptr;

    QLabel* m_next_time_step_label           = nullptr;
    QSpinBox* m_next_time_step_spin_box      = nullptr;

    QLabel* m_min_limit_time_step_label      = nullptr;
    QSpinBox* m_min_limit_time_step_spin_box = nullptr;

    QLabel* m_max_limit_time_step_label      = nullptr;
    QSpinBox* m_max_limit_time_step_spin_box = nullptr;

    QLabel* m_total_time_step_range_label    = nullptr;

    QLabel* m_update_interval_label          = nullptr;
    QSpinBox* m_update_interval_spin_box     = nullptr;

    WebSocketPair* m_web_sockets = nullptr;

    bool m_is_operator           = true;

    QTimer m_timer;
    bool m_is_reverse_mode  = false;
    bool m_is_last_mode     = false;
    bool m_is_merging       = false;

    void decrementTimeStep();
    void incrementTimeStep();
    void keepLastTimeStep();

private slots:
    void onUpdateNext( const int next );
    void onUpdateMinLimit( const int minLimit );
    void onUpdateMaxLimit( const int maxLimit );
    void onUpdateInterval( const int interval );
    void onUpdateTimeStep();
};

#endif // TIMESTEPCONTROLTOOLBAR_H
