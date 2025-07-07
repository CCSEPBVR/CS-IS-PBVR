/*****************************************************************************/
/**
 *  @file   OpenXRInteractor.h
 *  @author Keisuke Tajiri
 */
/*****************************************************************************/
#pragma once
#include <kvs/InteractorBase>
#include <kvs/LineObject>
#include <kvs/PolygonImporter>
#include <SupportOpenXR/Viewer/Screen.h>
#include <SupportOpenXR/RemoteDesktop/RemoteDesktop.h>

namespace kvs
{

namespace openxr
{

class Screen;

/*===========================================================================*/
/**
 *  @brief  OpenXR interactor class.
 */
/*===========================================================================*/
class OpenXRInteractor : public kvs::InteractorBase
{
public:
    using BaseClass = kvs::InteractorBase;

private:
    kvs::openxr::Screen* m_openxr_screen = nullptr;
    kvs::openxr::RemoteDesktop* m_remote_desktop = nullptr;
    kvs::LineObject* m_axis_x = nullptr;
    kvs::LineObject* m_axis_y = nullptr;
    kvs::LineObject* m_axis_z = nullptr;
    kvs::LineObject* m_pointer = nullptr;
    kvs::Vec3 m_pointer_scale = kvs::Vec3::Ones();
    kvs::PolygonObject* m_controller_model[kvs::Side::Max] = { nullptr };
    kvs::Timer m_elapsed_timer = {};

public:
    OpenXRInteractor( kvs::openxr::Screen* screen );
    virtual ~OpenXRInteractor();

protected:
    virtual void mousePressEvent( kvs::MouseEvent* e ) {}
    virtual void mouseMoveEvent( kvs::MouseEvent* e ) {}
    virtual void mouseReleaseEvent( kvs::MouseEvent* e ) {}
    virtual void wheelEvent( kvs::WheelEvent* e ) {}
    virtual void keyPressEvent( kvs::KeyEvent* e ) {}
    virtual void initializeEvent();
    virtual void paintEvent();
    virtual void timerEvent( kvs::TimeEvent* e );
    virtual void controllerPressEvent( kvs::ControllerEvent* e );
    virtual void controllerReleaseEvent( kvs::ControllerEvent* e );
    virtual void controllerMoveEvent( kvs::ControllerEvent* e );
    virtual void controllerAxisEvent( kvs::ControllerEvent* e );
};

} // end of namespace openxr

} // end of namespace kvs
