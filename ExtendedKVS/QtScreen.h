#ifndef QTSCREEN_H
#define QTSCREEN_H

#include <kvs/qt/Screen>

class QtScreen : public kvs::qt::Screen
{
public:
    QtScreen( kvs::qt::Application* application = 0, QWidget* parent = 0 );

    virtual void mouseReleaseEvent( kvs::MouseEvent* event ){ Q_UNUSED( event ); }
    virtual void wheelEvent( kvs::WheelEvent* event ){ Q_UNUSED( event ); }
    virtual void keyPressEvent( kvs::KeyEvent* event );
};

#endif // QTSCREEN_H
