#ifndef ANIMATIONCONTROLS_H
#define ANIMATIONCONTROLS_H

#include <QDockWidget>

namespace Ui {
class AnimationControls;
}

class AnimationControls : public QDockWidget
{
    Q_OBJECT

public:
    explicit AnimationControls(QWidget *parent = nullptr);
    ~AnimationControls();

private:
    Ui::AnimationControls *ui;
};

#endif // ANIMATIONCONTROLS_H
