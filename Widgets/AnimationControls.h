#ifndef ANIMATIONCONTROLS_H
#define ANIMATIONCONTROLS_H

#include <QDockWidget>
#include <kvs/Xform>

namespace Ui {
class AnimationControls;
}

class AnimationControls : public QDockWidget
{
    Q_OBJECT

public:
    explicit AnimationControls(QWidget *parent = nullptr);
    ~AnimationControls();

    void InitializeKeyFrame();
    void addKeyFrameAdd( kvs::Xform xform ); //x
    void removeLasrKeyFrame();               //d
    void clearKeyFrame();                    //D
    void playKeyFrame();                     //M
    void loadKeyFrameFile();                 //L
    void saveKeyFrameFile();                 //S

private:
    Ui::AnimationControls *ui;
    QVector<kvs::Xform> m_xforms;

};

#endif // ANIMATIONCONTROLS_H
