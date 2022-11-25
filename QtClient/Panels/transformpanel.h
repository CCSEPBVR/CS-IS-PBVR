#ifndef TRANSFORMPANEL_H
#define TRANSFORMPANEL_H

#include <QDockWidget>
#include <QGlue/extCommand.h>
#include <Client/v3defines.h>

typedef struct _TransformParam
{
      float rotationAxisX = PBVR_ROTATION_X_AXIS_LEVEL;
      float rotationAxisY = PBVR_ROTATION_Y_AXIS_LEVEL;
      float rotationAxisZ = PBVR_ROTATION_Z_AXIS_LEVEL;
      float translateX    = PBVR_TRANSLATE_X_LEVEL;
      float translateY    = PBVR_TRANSLATE_Y_LEVEL;
      float translateZ    = PBVR_TRANSLATE_Z_LEVEL;
      float scalingLevel  = PBVR_SCALING_LEVEL;

}TransformParam;

namespace Ui {
class TransformPanel;
}

class TransformPanel : public QDockWidget
{

public:
    explicit TransformPanel(QWidget *parent = 0);
    ~TransformPanel();

public:

    static TransformParam param;

    Qt::DockWidgetArea default_area=Qt::RightDockWidgetArea;

    static void setRotationAxisX( char* c );
    static void setRotationAxisY( char* c );
    static void setRotationAxisZ( char* c );
    static void setTranslationX( char* c);
    static void setTranslationY( char* c );
    static void setTranslationZ( char* c);
    static void setScaling( char* c );

private://Event Handlers

    void onTranslationXchanged(double val);
    void onTranslationYchanged(double val);
    void onTranslationZchanged(double val);
    void onXaxisRotationChanged(double val);
    void onYaxisRotationChanged(double val);
    void onZaxisRotationChanged(double val);
    void onUniformScaleChanged(double val);
    void onApplyButtonClicked();

private:
    void customEvent(QEvent * event);
    static TransformPanel* instance;
    Ui::TransformPanel *ui;
};

#endif // TRANSFORMPANEL_H
