#ifndef PBVRCLIENT_H
#define PBVRCLIENT_H

#include <QMainWindow>
#include <QDockWidget>
#include <QSettings>

#include "Panels/filterinfopanel.h"
#include "Panels/coordinatepanel.h"
#include "Panels/legendpanel.h"
#include "Panels/particlepanel.h"
#include "Panels/renderoptionpanel.h"
#include "Panels/systemstatuspanel.h"
#include "Panels/timecontrolpanel.h"
#include "Panels/transformpanel.h"
#include "Panels/animationcontrols.h"
#include "Panels/transferfunceditor.h"
#include "Panels/viewercontrols.h"
#include "helpdialog.h"

#include "QGlue/renderarea.h"


namespace Ui {
class PBVRGUI;
}

/**
 * @brief
 *
 */
class PBVRGUI : public QMainWindow
{
public:

    explicit PBVRGUI(ExtCommand *command);
    ~PBVRGUI();

    AnimationControls animationControls;

    TransferFuncEditor tf_editor;
    CoordinatePanel coordinatePanel;
    FilterinfoPanel filterinfoPanel;
    LegendPanel legendPanel;
    ParticlePanel particlePanel;
    RenderoptionPanel renderoptionPanel;
    SystemstatusPanel systemstatusPanel;
    TimecontrolPanel timecontrolPanel;
    TransformPanel transformPanel;
    HelpDialog helpdialog;
    ViewerControls vc;
    RenderArea* kvs_renderarea;



    void start();
    void setup_viewer_settings();
    void load_parameter_file(kvs::visclient::Argument&argument);

private:
    Ui::PBVRGUI *ui;
    QSettings* gui_settings;

    bool dockstate_restored=false;

    void closeEvent(QCloseEvent *event);
    void restoreDefaultLayout();
    int  restoreAllState();
    void restoreDockState(QDockWidget* dock);
    void showStatusMessage(QString message, bool showMsgBox=false);
    void showDock(QDockWidget* dock,Qt::DockWidgetArea dock_area);
    void setBackgroundColor(kvs::RGBColor c);

    //Menu item event handlers
private:
    void onTransfer_Function_EditorMenuAction();
    void onFilter_InformationMenuAction();
    void onConnect_To_ServerMenuAction();
    void onDisconnect_From_ServerMenuAction();
    void onExportMenuAction();
    void onAbout_PBVR_ClientMenuAction();
    void onImportMenuAction();
    void onImport_Transfer_FunctionsMenuAction();
    void onExport_Transfer_FunctionsMenuAction();
    void onSystem_StatusMenuAction();
    void onRender_OptionsMenuAction();
    void onVolume_TransformMenuAction();
    void onTime_ControlsMenuAction();
    void onLegend_OptionsMenuAction();
    void onParticle_PanelMenuAction();
    void onCoordinatesMenuAction();
    void onPBVR_Client_ManualMenuAction();
    void onRevert_to_Default_LayoutMenuAction();
    void onFull_ScreenMenuAction();
    void onAnimation_ControlsMenuAction();
    void onViewer_ControlsMenuAction();


protected:
    void keyPressEvent(QKeyEvent *ke);
//    ADD BY)T.Osaki 2020.03.03
    void mousePressEvent(QMouseEvent *event);

};

#endif // PBVRCLIENT_H
