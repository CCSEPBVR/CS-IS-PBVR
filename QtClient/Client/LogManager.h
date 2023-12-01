#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <App/helpdialog.h>
#include <QGlue/renderarea.h>
#include <Panels/animationcontrols.h>
#include <Panels/coordinatepanel.h>
#include <Panels/filterinfopanel.h>
#include <Panels/legendpanel.h>
#include <Panels/particlepanel.h>
#include <Panels/renderoptionpanel.h>
#include <Panels/systemstatuspanel.h>
#include <Panels/timecontrolpanel.h>
#include <Panels/transferfunceditor.h>
#include <Panels/transformpanel.h>
#include <Panels/viewercontrols.h>

#include <QGlue/renderarea.h>
#include <iostream>
#include <kvs/Xform>

class LogManager
{
public:
    LogManager();
    void Write(std::string className, std::string funcName, std::string comment, std::string data);
    std::vector<std::string> split(std::string str, char del);
    void trace();
    void check(std::string className, std::string funcName, std::string data);

    void setScreen(RenderArea* screen){m_screen=screen;}
    void setAnimationControls(AnimationControls* animationControls){m_animationControls = animationControls;}
    void setTransferFuncEditor(TransferFuncEditor* tf_editor){m_tf_editor = tf_editor;}
    void setCoordinatePanel(CoordinatePanel* coordinatePanel){m_coordinatePanel = coordinatePanel;}
    void setFilterinfoPanel(FilterinfoPanel* filterinfoPanel){m_filterinfoPanel = filterinfoPanel;}
    void setLegendPanel(LegendPanel* legendPanel){m_legendPanel = legendPanel;}
    void setParticlePanel(ParticlePanel* particlePanel){m_particlePanel = particlePanel;};
    void setRenderoptionPanel(RenderoptionPanel* renderoptionPanel){m_renderoptionPanel = renderoptionPanel;}
    void setSystemstatusPanel(SystemstatusPanel* systemstatusPanel){m_systemstatusPanel = systemstatusPanel;}
    void setTimecontrolPanel(TimecontrolPanel* timecontrolPanel){m_timecontrolPanel = timecontrolPanel;}
    void setTransformPanel(TransformPanel* transformPanel){m_transformPanel = transformPanel;}
    void setViewerControls(ViewerControls* vc){m_vc = vc;}

    enum panelClass
    {
        RenderAreaP             = 0,
        AnimationControlsP      = 1,
        TransferFunctionEditorP = 2,
        CoordinateP             = 3,
        FilterinfoP             = 4,
        LegendP                 = 5,
        ParticleP               = 6,
        RenderoptionP           = 7,
        SystemstatusP           = 8,
        TimecontralP            = 9,
        TransformP              = 10,
        ViewerControlsP         = 11,
    };

private:
    RenderArea* m_screen;
    AnimationControls* m_animationControls;
    TransferFuncEditor* m_tf_editor;
    CoordinatePanel* m_coordinatePanel;
    FilterinfoPanel* m_filterinfoPanel;
    LegendPanel* m_legendPanel;
    ParticlePanel* m_particlePanel;
    RenderoptionPanel* m_renderoptionPanel;
    SystemstatusPanel* m_systemstatusPanel;
    TimecontrolPanel* m_timecontrolPanel;
    TransformPanel* m_transformPanel;
    ViewerControls* m_vc;
};

#endif // LOGMANAGER_H
