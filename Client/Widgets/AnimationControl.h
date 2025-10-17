#ifndef ANIMATIONCONTROL_H
#define ANIMATIONCONTROL_H

#include <QDockWidget>
#include <QFileDialog>

#include "Screen.h"

namespace Ui
{
class AnimationControl;
}

class AnimationControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit AnimationControl( kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~AnimationControl();

private:
    // メンバ変数群
    Ui::AnimationControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    QTimer* m_animation_timer;
    bool m_animation_paused;
    QVector<kvs::Xform> m_xforms;
    int m_xform_index = 0;
    int m_interpolation_counter = 0;

    // メソッド群
    void initialize();
    kvs::Xform InterpolateXform( const int interpolationStep, const int numberOfFrame, const kvs::Xform& start, const kvs::Xform& end );
    kvs::Quaternion RtoQ( const kvs::Matrix33f& R );
    float Sign( const float x );
    float Norm( const float a, const float b, const float c, const float d );

private slots:
    void onInterpolationValueChanged();

public slots:
    void addKeyFrameAdd( kvs::Xform xform );    // x
    void removeLastKeyFrame();                  // d
    void clearKeyFrame();                       // D
    void playKeyFrame();                        // M
    void loadKeyFrameFile();                    // L
    void saveKeyFrameFile();                    // S
    void screenShot( int loopCounter );         // ?
};
#endif // ANIMATIONCONTROL_H
