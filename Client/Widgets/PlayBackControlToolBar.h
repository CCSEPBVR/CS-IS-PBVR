#ifndef PLAYBACKCONTROLTOOLBAR_H
#define PLAYBACKCONTROLTOOLBAR_H

#include <QToolBar>
#include <QPushButton>
#include <QHBoxLayout>
#include <QElapsedTimer>

#include "WebSocketPair.h"

class PlayBackControlToolBar : public QToolBar
{
    Q_OBJECT

public:
    PlayBackControlToolBar( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~PlayBackControlToolBar();
    void reset();
    QPushButton* jumpButton() const { return m_jump_push_button; }

signals:
    void first();
    void previous();
    void reverse( bool isChecked );
    void play( bool isChecked );
    void next();
    void last();
    void keepLast( bool isChecked );
    void jump();
    void loop( bool isChecked );
    void renderCompletionRequested( quint64 measurementId );

public slots:
    void onOperatorStateUpdate( const bool operatorState ); // true: 権限有り, false: 権限無し
    void onDataRequestCompleted();
    void onRenderCompleted( quint64 measurementId );

private:
    QPushButton* m_first_push_button     = nullptr;
    QPushButton* m_previous_push_button  = nullptr;
    QPushButton* m_reverse_push_button   = nullptr;
    QPushButton* m_play_push_button      = nullptr;
    QPushButton* m_next_push_button      = nullptr;
    QPushButton* m_last_push_button      = nullptr;
    QPushButton* m_keep_last_push_button = nullptr;
    QPushButton* m_jump_push_button      = nullptr;
    QPushButton* m_loop_push_button      = nullptr;

    QVector<QPushButton*> m_buttons;

    WebSocketPair* m_web_sockets = nullptr;

    bool m_is_operator           = true;
    QElapsedTimer m_next_update_timer;
    quint64 m_next_measurement_id = 0;
    bool m_waiting_for_render = false;

    QPushButton* createPushButton( const QString& iconPath, const QSize& iconSize, const QSize& buttonSize, QWidget* parent );
    void disableButtons();
    void enableButtons();

private slots:
    void onFirst();
    void onPrevious();
    void onReverse();
    void onPlay();
    void onNext();
    void onLast();
    void onKeepLast();
    void onJump();
    void onLoop();
};

#endif // PLAYBACKCONTROLTOOLBAR_H
