#ifndef SCREEN_H
#define SCREEN_H

#ifdef DESKTOP_SCREEN_MODE
#include <kvs/qt/Screen>
#elif OPENXR_SCREEN_MODE
#include <kvs/openxr/Screen>
#endif

namespace kvs
{

namespace qt
{

namespace jaea
{

#ifdef DESKTOP_SCREEN_MODE
class Screen : public kvs::qt::Screen
#elif OPENXR_SCREEN_MODE
class Screen : public kvs::openxr::Screen
#endif
{
public:
    Screen( kvs::qt::Application* application = 0, QWidget* parent = 0 );

    virtual void mouseReleaseEvent( kvs::MouseEvent* event )  override { Q_UNUSED( event ); }
    virtual void wheelEvent( kvs::WheelEvent* event ) override { Q_UNUSED( event ); }
    virtual void keyPressEvent( kvs::KeyEvent* event ) override { Q_UNUSED( event ); };
};

} // end of namespace jaea

} // end of namespace qt

} // end of namespace kvs
#endif // SCREEN_H
