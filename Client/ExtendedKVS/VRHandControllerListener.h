#ifndef VRHANDCONTROLLERLISTENER_H
#define VRHANDCONTROLLERLISTENER_H

#include <kvs/EventListener>
#include "Screen.h"

class VRHandControllerListener : public QObject, public kvs::EventListener
{
    Q_OBJECT
public:
    explicit VRHandControllerListener( kvs::qt::jaea::Screen* screen );
    ~VRHandControllerListener();

    void onEvent( kvs::EventBase* event ) override;

signals:
    void drawPlotOverLine( kvs::Real32 CoordArray[ 2 * 3 ] );
    void showHidePlotOverLine();
private:
    kvs::qt::jaea::Screen* m_screen       = nullptr;
};

#endif // VRHANDCONTROLLERLISTENER_H
