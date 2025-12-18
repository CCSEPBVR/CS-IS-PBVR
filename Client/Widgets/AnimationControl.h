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

public slots:
    void onAddKeyFrameAdd( kvs::Xform xform ); // x
    void onRemoveLastKeyFrame();               // d
    void onClearKeyFrame();                    // D
    void onPlayKeyFrame();                     // M
    void onLoadKeyFrameFile();                 // L
    void onSaveKeyFrameFile();                 // S
    void onScreenShot( int loopCounter );      // ?
    // FIXME:KPI
    void onLoadParameter( const QString& filePath );
    void onSaveParameter( const QString& filePath );

private:
    Ui::AnimationControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    QTimer* m_animation_timer;
    bool m_animation_paused;
    QVector<kvs::Xform> m_xforms;
    int m_xform_index;
    int m_interpolation_counter;

    kvs::Xform InterpolateXform( const int interpolationStep, const int numberOfFrame, const kvs::Xform& start, const kvs::Xform& end );
    kvs::Quaternion RtoQ( const kvs::Matrix33f& R );
    float Sign( const float x );
    float Norm( const float a, const float b, const float c, const float d );

private slots:
    void onInterpolationValueChanged();
};

#endif // ANIMATIONCONTROL_H
