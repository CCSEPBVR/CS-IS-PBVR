#ifndef RENDEROPTIONPANEL_H
#define RENDEROPTIONPANEL_H

#include <QDockWidget>
#include <QGlue/extCommand.h>
#include "boundingboxsynchronizer.h"

namespace Ui {
class RenderoptionPanel;
}

//class RenderoptionPanel : public QDockWidget, public BoundingBoxSynchronizerClient
class RenderoptionPanel : public QDockWidget
{

public:

    explicit RenderoptionPanel(QWidget *parent = 0);
    void updateCmd2UI();
    ~RenderoptionPanel();

    void logPlay(std::string funcName, std::string str[]);
    void recToggle(){ m_isRec ? m_isRec = false : m_isRec = true; }

    Qt::DockWidgetArea default_area=Qt::RightDockWidgetArea;
    float pixelRatio = 1;
    static float data_size_limit;    // = PBVR_RESOLUTION_WIDTH_LEVEL;
    static float resolution_width_level;    // = PBVR_RESOLUTION_WIDTH_LEVEL;
    static float resolution_height_level;   // = PBVR_RESOLUTION_HEIGHT_LEVEL;
    static float pdensitylevel;             // = PBVR_PARTICLE_DENSITY;
    static int   plimitlevel;               // = PBVR_PARTICLE_LIMIT;
//    static bool  isEnabledBoundingBox;      // = PBVR_IS_ENABLED_BOUNDING_BOX;

    static void setResolutionWidth( char* c){
        resolution_width_level=atof(c);
    }
    static void setResolutionHeight( char* c){
        resolution_height_level=atof(c);
    }

    static void setParticleDensity( char* c){
        pdensitylevel=atof(c);
    }
    static void setParticleLimit( char* c){
        plimitlevel=atof(c);
    }
//    static void setIsEnabledBoundingBox( bool isEnabled){
//        isEnabledBoundingBox = isEnabled;
//    }
    void updateBoundingBoxVisibility(bool isVisible);

private:
    void onDataSizeLimitChanged(int val);
    void onParticleDensityChanged(double val);
    void onParticleLimitChanged(int val);
    void onReslutionHeightChanged(int val);
    void onResolutionWidthChanged(int val);
//    void onIsEnabledBoundingBoxChanged(bool);
    void onApplyButtonClicked();

private:
    Ui::RenderoptionPanel *ui;
    bool m_isRec = false;
};

#endif // RENDEROPTIONPANEL_H
