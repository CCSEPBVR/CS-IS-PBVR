#ifndef ANIMATIONCONTROLS_H
#define ANIMATIONCONTROLS_H

#include <QDockWidget>

#include "ExtendedKVS/Screen.h"
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

    void setScreen( kvs::qt::jaea::Screen* screen ) { m_screen = screen; }
    void InitializeKeyFrame();
    void addKeyFrameAdd( kvs::Xform xform ); //x
    void removeLasrKeyFrame();               //d
    void clearKeyFrame();                    //D
    void playKeyFrame();                     //M
    void loadKeyFrameFile();                 //L
    void saveKeyFrameFile();                 //S

private:
    Ui::AnimationControls *ui;
    kvs::qt::jaea::Screen* m_screen;
    QVector<kvs::Xform> m_xforms;
    int m_xform_index = 0;
    int m_ninterpolation;
    int m_interpolation_counter = 0;

    kvs::Xform InterpolateXform( const int interp_step, const int num_frame, const kvs::Xform& start, const kvs::Xform& end );
    kvs::Quaternion RtoQ( const kvs::Matrix33f& R );
    float Sign( const float x );
    float Norm( const float a, const float b, const float c, const float d );
};

#endif // ANIMATIONCONTROLS_H
