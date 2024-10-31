#ifndef SCREEN_H
#define SCREEN_H

#include <kvs/openxr/Screen>
#include <kvs/qt/Screen>

namespace kvs
{

namespace qt
{

namespace jaea
{

class Screen : public kvs::openxr::Screen //OpenXR
// class Screen : public kvs::qt::Screen
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
