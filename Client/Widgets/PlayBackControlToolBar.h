#ifndef PLAYBACKCONTROLTOOLBAR_H
#define PLAYBACKCONTROLTOOLBAR_H

#include <QToolBar>
#include <QHBoxLayout>
#include <QPushButton>

class PlayBackControlToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit PlayBackControlToolBar( QWidget *parent = nullptr );
    ~PlayBackControlToolBar();

public slots:
    void donePlayBackControlToolBar();

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
    QPushButton* m_first_push_button;
    QPushButton* m_previous_push_button;
    QPushButton* m_reverse_push_button;
    QPushButton* m_play_push_button;
    QPushButton* m_next_push_button;
    QPushButton* m_last_push_button;
    QPushButton* m_keep_last_push_button;
    QPushButton* m_jump_push_button;
    QPushButton* m_loop_push_button;

    void initialize();
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
