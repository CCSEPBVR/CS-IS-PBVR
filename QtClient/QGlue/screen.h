#pragma once
#ifndef KVSQS2creen_H
#define KVSQS2creen_H
#include <GL/glew.h>
#include <kvs/OpenGL>
#include <QThread>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <kvs/ScreenBase>
//#include <kvs/Scene>

#include <OVR_CAPI.h>
#include "headmounteddisplay.h"
#include "pbvrwidgethandler.h"
#include "Scene.h"
#include "TouchController.h"
#include "PBVRStochasticRenderingCompositor.h"

#define V_INVERSE_IS_NOT_INITIALIZED -1
#define V_INVERSE_IS_INITIALIZING     0
#define V_INVERSE_IS_INITIALIZED      1

namespace kvs {
namespace oculus {
namespace jaea {
    class TouchController;
    class Scene;
}
}
}
class PBVRStochasticRenderingCompositor;

/**
 * @brief The Screen class, this class is designed as a combined and simplified version
 *        of the kvs::qt::ScreenBase, and   kvs::qt::Screen. This class uses Qt events directly
 *        and does not translate QtEvents to kvs::event class.
 *
 *        This class is desgined to work with  QOpenGLWidget as a baseclass, whereas
 *        in kvs 2.9.0, QOpenGLWidget is not supported.
 *
 *        It does however require a version of KVS which includes commit
 *        https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
 */
class Screen :public kvs::ScreenBase,public QOpenGLWidget, public QOpenGLFunctions
{

public:
    static const kvs::Vec3 KVS_CAMERA_INITIAL_POSITION;

    typedef kvs::ScreenBase BaseClass;
//    typedef kvs::oculus::jaea::Scene::ControlTarget ControlTarget;
    bool m_hold_paintGL=false;
    double m_fps = 0.0;

//    // from OculusKVS (kvs::oculus::kaea::Screen)
//    enum MirrorBufferType
//    {
//        LeftEyeImage = 0,
//        RightEyeImage,
//        BothEyeImage,
//        DistortedBothEyeImage
//    };

    enum ManipulatorUseFlag
    {
        UseNoManipulator = 0,
        UseLeftManipulator,
        UseRightManipulator,
    };


private:
    bool m_enable_default_paint_event;

    // from OculusKVS (kvs::oculus::kaea::Screen)
    kvs::oculus::jaea::HeadMountedDisplay m_hmd; ///< Oculus HMD
    //MirrorBufferType m_mirror_buffer_type; ///< mirror buffer type

    kvs::oculus::jaea::TouchController *m_touch_controller;
    kvs::oculus::jaea::WidgetHandler *m_widget_handler;
    //ManipulatorUseFlag m_enableManipulator;

    //ObjectSelectionManager m_objectSelectionManager;

#ifdef ENABLE_ORIENTATION_CORRECTION
    int m_v_inverse_initialized;
    kvs::Xform m_v_inverse[2];

#endif // ENABLE_ORIENTATION_CORRECTION

protected:
//    kvs::Scene* m_scene;
    kvs::oculus::jaea::Scene* m_scene;
    bool m_gl_initialized;
    PBVRStochasticRenderingCompositor *m_compositor;

protected:
    virtual void defaultPaintEvent();


public:
    Screen( QWidget* parent_surface = NULL);
    ~Screen();
    kvs::oculus::jaea::Scene* scene(){return m_scene;}

    void setPosition( const int x, const int y );
    void setGeometry( const int x, const int y, const int width, const int height );

    void paintGL();
    void resizeGL(int width, int height);
    void initializeGL();

    virtual void onPaintGL(){}
    virtual void onResizeGL(int width, int height){}
    virtual void onInitializeGL(){}

    void update()
    {
        if (QThread::currentThread() != this->thread()) {
            // Called from different thread do nothing
            return;
        }
        QOpenGLWidget::update();
    }

    void redraw()
    {
        this->update();
        //assert(false);
    }

    // from OculusKVS (kvs::oculus::jaea::Screen)
    //bool isEnabledManipulator();
    //void setEnabledManipulator(ManipulatorUseFlag flag);

    void setWidgetHandler (kvs::oculus::jaea::WidgetHandler *handler);

    const kvs::oculus::jaea::HeadMountedDisplay& headMountedDisplay() const { return m_hmd; }
//    void setMirrorBufferType( const MirrorBufferType type ) { m_mirror_buffer_type = type; }
//    void setMirrorBufferTypeToLeftEyeImage() { this->setMirrorBufferType( LeftEyeImage ); }
//    void setMirrorBufferTypeToRightEyeImage() { this->setMirrorBufferType( RightEyeImage ); }
//    void setMirrorBufferTypeToBothEyeImage() { this->setMirrorBufferType( BothEyeImage ); }
//    void setMirrorBufferTypeToDistortedBothEyeImage() { this->setMirrorBufferType( DistortedBothEyeImage ); }
    void setMirrorBufferTypeToLeftEyeImage() { m_hmd.setMirrorBufferTypeToLeftEyeImage(); }
    void setMirrorBufferTypeToRightEyeImage() { m_hmd.setMirrorBufferTypeToRightEyeImage(); }
    void setMirrorBufferTypeToBothEyeImage() { m_hmd.setMirrorBufferTypeToBothEyeImage(); }
    void setMirrorBufferTypeToDistortedBothEyeImage() { m_hmd.setMirrorBufferTypeToDistortedBothEyeImage(); }

};

#endif // KVSQScreen_H
