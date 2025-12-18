#ifndef PLAYBACKCONTROLTOOLBAR_H
#define PLAYBACKCONTROLTOOLBAR_H

#include <QToolBar>
#include <QPushButton>
#include <QHBoxLayout>

#include "WebSocketPair.h"

class PlayBackControlToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit PlayBackControlToolBar( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~PlayBackControlToolBar();

public slots:
    void onUpdateServerState( bool serverState );     // true:接続中
    void onOperatorStateUpdate( bool operatorState ); // true:権限あり
    void onReset();
    void onDataRequestCompleted();

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

private:
    WebSocketPair* m_web_sockets = nullptr;
    bool m_is_operator;

    QPushButton* m_first_push_button        = nullptr;
    QPushButton* m_previous_push_button     = nullptr;
    QPushButton* m_reverse_push_button      = nullptr;
    QPushButton* m_play_push_button         = nullptr;
    QPushButton* m_next_push_button         = nullptr;
    QPushButton* m_last_push_button         = nullptr;
    QPushButton* m_keep_last_push_button    = nullptr;
    QPushButton* m_jump_push_button         = nullptr;
    QPushButton* m_loop_push_button         = nullptr;

    QVector<QPushButton*> m_buttons;

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
