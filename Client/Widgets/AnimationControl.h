#ifndef ANIMATIONCONTROL_H
#define ANIMATIONCONTROL_H

#include <QDockWidget>

namespace Ui
{
class AnimationControl;
}

class AnimationControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit AnimationControl( QWidget *parent = nullptr );
    ~AnimationControl();

private:
    Ui::AnimationControl *ui;

    void initialize();
};
#endif // ANIMATIONCONTROL_H
