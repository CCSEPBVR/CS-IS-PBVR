#ifndef PLAYBACKCONTROLTOOLBARWIP_H
#define PLAYBACKCONTROLTOOLBARWIP_H

#include <QToolBar>
#include <QPushButton>
#include <QHBoxLayout>

class PlayBackControlToolBarWIP : public QToolBar
{
    Q_OBJECT
public:
    PlayBackControlToolBarWIP( QWidget *parent = nullptr );
    ~PlayBackControlToolBarWIP();

public slots:
    void onDataRequestCompleted();
    void onOperatorStateUpdate( bool operatorState );

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

#endif // PLAYBACKCONTROLTOOLBARWIP_H
