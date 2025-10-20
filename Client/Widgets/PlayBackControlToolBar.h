#ifndef PLAYBACKCONTROLTOOLBAR_H
#define PLAYBACKCONTROLTOOLBAR_H

#include <QToolBar>
#include <QPushButton>
#include <QHBoxLayout>

class PlayBackControlToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit PlayBackControlToolBar( QWidget *parent = nullptr );
    ~PlayBackControlToolBar();

private:
    // メンバ変数群
    QPushButton *m_first_time_step_push_button;
    QPushButton *m_previous_time_step_push_button;
    QPushButton *m_reverse_push_button;
    QPushButton *m_play_push_button;
    QPushButton *m_next_time_step_push_button;
    QPushButton *m_last_time_step_push_button;
    QPushButton *m_keep_last_time_step_push_button;
    QPushButton *m_jump_push_button;
    QPushButton *m_loop_push_button;

    // メソッド群
    void initialize();
    QPushButton* createButton( const QString& iconPath, const QSize& iconSize, const QSize& buttonSize, QWidget* parent );
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

public slots:
    void doneMerge();
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

signals:
    void fisrtTimeStep();
    void previousTimeStep();
    void reverseTimeStep( bool isChecked );
    void playTimeStep( bool isChecked );
    void nextTimeStep();
    void lastTimeStep();
    void keepLastTimeStep( bool isChecked );
    void jumpTimeStep();
    void loopMode( bool isChecked );
};

#endif // PLAYBACKCONTROLTOOLBAR_H
