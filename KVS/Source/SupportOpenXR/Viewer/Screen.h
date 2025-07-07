/*****************************************************************************/
/**
 *  @file   Screen.h
 *  @author Keisuke Tajiri
 */
/*****************************************************************************/
#pragma once

#include <SupportOpenXR/OpenXR/OpenXRDevice.h>
#include <SupportOpenXR/OpenXR/OpenXRInteractor.h>
#include <SupportQt/Viewer/ScreenBase.h>
#include <kvs/Scene>
#include <kvs/Mouse>
#include <kvs/RGBColor>
#include <kvs/ColorImage>
#include <kvs/InteractorBase>
#include <kvs/ControllerEvent>


namespace kvs
{

class ObjectBase;
class RendererBase;
class VisualizationPipeline;

namespace openxr
{

class Application;
class OpenXRInteractor;

/*===========================================================================*/
/**
 *  @brief  OpenXR screen class.
 */
/*===========================================================================*/
class Screen : public kvs::qt::ScreenBase
{
    Q_OBJECT

public:
    using BaseClass = kvs::qt::ScreenBase;
    using ControlTarget = kvs::Scene::ControlTarget;
    using ColorToDepthMap = std::map<GLuint, GLuint>;

    static Screen* DownCast( kvs::ScreenBase* screen );
    static const Screen* DownCast( const kvs::ScreenBase* screen );

private:
    kvs::Scene* m_scene = nullptr;
    kvs::InteractorBase* m_interactor = nullptr;
    kvs::openxr::OpenXRInteractor* m_openxr_interactor = nullptr;
    kvs::ControllerEvent* m_controller_event = nullptr;
    kvs::openxr::OpenXRDevice*  m_openxr_device = nullptr;
    kvs::Xform m_head_xform = kvs::Xform();
    kvs::Xform m_walkthrough_xform = kvs::Xform();
    GLuint m_eye_fbo[kvs::Side::Max] = { 0 };
    ColorToDepthMap m_color_to_depth_map;
    kvs::UInt32 m_ops_hand = kvs::Side::Right;
    kvs::Timer m_fps_timer = {};

public:
    Screen( kvs::qt::Application* application = 0, QWidget* parent = 0 );
    virtual ~Screen();

    kvs::Scene* scene() { return m_scene; }

    const kvs::Xform& headXform() const { return m_head_xform; }
    const kvs::Xform&  walkthrough() const { return m_walkthrough_xform; }
    void setWalkthrough( const kvs::Xform& walkthrough ) { m_walkthrough_xform = walkthrough; }
    bool opsHand() const { return m_ops_hand; }
    void setOpsHand( kvs::UInt32 ops_hand ) { m_ops_hand = ops_hand; }

    void setTitle( const std::string& title );
    void setPosition( const int x, const int y );
    void setSize( const int width, const int height );
    void setGeometry( const int x, const int y, const int width, const int height );
    void setBackgroundColor( const kvs::RGBColor& color );
    void setBackgroundColor( const kvs::RGBColor& color1, const kvs::RGBColor& color2 );
    void setBackgroundImage( const kvs::ColorImage& image );
    void setControlTarget( const ControlTarget target );
    void setControlTargetToObject();
    void setControlTargetToCamera();
    void setControlTargetToLight();

    const std::pair<int, int> registerObject( kvs::ObjectBase* object, kvs::RendererBase* renderer = 0 );
    const std::pair<int, int> registerObject( kvs::VisualizationPipeline* pipeline );

    virtual void setEvent( kvs::EventListener* event, const std::string& name = "" );
    virtual void addEvent( kvs::EventListener* event, const std::string& name = "" );

    virtual void enable();
    virtual void disable();
    virtual void reset();

    virtual void initializeEvent();
    virtual void paintEvent();
    virtual void resizeEvent( int width, int height );
    virtual void mousePressEvent( kvs::MouseEvent* event );
    virtual void mouseMoveEvent( kvs::MouseEvent* event );
    virtual void mouseReleaseEvent( kvs::MouseEvent* event );
    virtual void mouseDoubleClickEvent( kvs::MouseEvent* event );
    virtual void wheelEvent( kvs::WheelEvent* event );
    virtual void keyPressEvent( kvs::KeyEvent* event );
    virtual void keyRepeatEvent( kvs::KeyEvent* event );
    virtual void keyReleaseEvent( kvs::KeyEvent* event );
    virtual void controllerPressEvent( kvs::ControllerEvent* event );
    virtual void controllerReleaseEvent( kvs::ControllerEvent* event );
    virtual void controllerMoveEvent( kvs::ControllerEvent* event );
    virtual void controllerAxisEvent( kvs::ControllerEvent* event );

private:
    GLuint getDepthTexture( GLuint color_texture );
};

} // end of namespace openxr

} // end of namespace kvs
