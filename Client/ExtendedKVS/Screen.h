#ifndef SCREEN_H
#define SCREEN_H

#include <kvs/qt/Screen>
#ifdef OPENXR_SCREEN
// #include <kvs/openxr/Screen>
#include "OpenXRScreen.h"
#endif
#include <kvs/ObjectManager>

namespace kvs
{

namespace qt
{

namespace jaea
{

#ifdef OPENXR_SCREEN
class Screen : public kvs::openxr::OpenXRScreen
#else
class Screen : public kvs::qt::Screen
#endif
{
    Q_OBJECT
public:
    Screen( kvs::qt::Application* application = 0, QWidget* parent = 0 );

    virtual void mouseReleaseEvent( kvs::MouseEvent* event )  override { Q_UNUSED( event ); }
    virtual void wheelEvent( kvs::WheelEvent* event ) override { Q_UNUSED( event ); }
    virtual void keyPressEvent( kvs::KeyEvent* event ) override;

signals:
    void updateTranslation();
    void addKeyFrameAdd( kvs::Xform xform ); // x
    void removeLastKeyFrame();               // d
    void clearKeyFrame();                    // D
    void playKeyFrame();                     // M
    void loadKeyFrameFile();                 // L
    void saveKeyFrameFile();                 // S
};

} // end of namespace jaea

} // end of namespace qt

} // end of namespace kvs

#endif // SCREEN_H
