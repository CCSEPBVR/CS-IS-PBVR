/**
 * @file OpenXRInteractor.h
 * @note OpenXR スクリーンに VR ハンドコントローラーでプロットオーバーラインを描画するために
 * 改造されたクラスです。元の実装は KVS/Source/SupportOpenXR/OpenXR/OpenXRInteractor に基づいています。
 */
#ifndef OPENXRINTERACTOR_H
#define OPENXRINTERACTOR_H
#include <kvs/InteractorBase>
#include <kvs/LineObject>
#include <kvs/PolygonImporter>
#include "OpenXRScreen.h"
#include <SupportOpenXR/RemoteDesktop/RemoteDesktop.h>
#include <kvs/PointObject>

namespace kvs
{

namespace openxr
{

class OpenXRScreen;

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
    kvs::openxr::OpenXRScreen* m_openxr_screen = nullptr;
    kvs::LineObject* m_axis_x = nullptr;
    kvs::LineObject* m_axis_y = nullptr;
    kvs::LineObject* m_axis_z = nullptr;
    kvs::LineObject* m_pointer = nullptr;
    kvs::Vec3 m_pointer_scale = kvs::Vec3::Ones();
    kvs::PolygonObject* m_controller_model[kvs::Side::Max] = { nullptr };
    kvs::Timer m_elapsed_timer = {};

    kvs::PointObject* m_start_point = nullptr;
    kvs::PointObject* m_end_point = nullptr;

    kvs::Vec3 m_start_initial_translation;
    kvs::Vec3 m_end_initial_translation;

public:
    OpenXRInteractor( kvs::openxr::OpenXRScreen* screen );
    virtual ~OpenXRInteractor();

    void setStartPoint( kvs::PointObject* startPoint ) { m_start_point = startPoint; }
    void setEndPoint( kvs::PointObject* endPoint ) { m_end_point = endPoint; }
    kvs::PointObject* startPoint() const { return m_start_point; }
    kvs::PointObject* endPoint() const { return m_end_point; }

    void setStartInitialTranslation( kvs::Vec3 startInitialTranslation ) { m_start_initial_translation = startInitialTranslation; }
    void setEndInitialTranslation( kvs::Vec3 endInitialTranslation ) { m_end_initial_translation = endInitialTranslation; }
    kvs::Vec3 startInitialTranslation() const { return m_start_initial_translation; }
    kvs::Vec3 endInitialTranslation() const { return m_end_initial_translation; }

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

#endif // OPENXRINTERACTOR_H
