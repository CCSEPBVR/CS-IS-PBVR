#include "pbvrgui.h"
#include "ui_pbvrgui.h"

#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QResource>
#include <QThread>
#include <QKeyEvent>

#include "Client/VizParameterFile.h"
#include "Client/ParticleMerger.h"

#include <kvs/RGBColor>
//KVS2.7.0
//ADD BY)T.Osaki 2020.06.06
#include <kvs/Background>
ExtCommand* extCommand;
/**
 * @brief PBVRClient::PBVRClient constructor
 * @param parent
 */
PBVRGUI::PBVRGUI(ExtCommand* command) :
    QMainWindow(Q_NULLPTR),
    ui(new Ui::PBVRGUI),
    particlePanel(this),
    filterinfoPanel(this),
    legendPanel(this),
    renderoptionPanel(this),
    systemstatusPanel(this),
    timecontrolPanel(this),
    transformPanel(this),
    animationControls(this),
    vc(this)
{

    if (command== NULL){
        qInfo("Application needs valid command object to start");
        exit(1);
    }
    extCommand=command;
    ui->setupUi(this);
    kvs_renderarea= ui->kvs_renderarea;

    showNormal();
        restoreAllState();
    setWindowTitle("PBVR Viewer");

#ifdef IS_MODE
    ui->actionExport->setVisible(false);
    ui->actionImport->setVisible(false);
    ui->actionCoordinates->setVisible(false);
#endif

    extCommand->m_particle_merge_ui=&particlePanel;
    extCommand->m_coord_panel_ui=&coordinatePanel;
    extCommand->m_timectrl_panel=&timecontrolPanel;
    extCommand->m_screen=kvs_renderarea;
    extCommand->extransfuncedit=&tf_editor;

    legendPanel.bindLegendBar(kvs_renderarea->g_legend);

    //Connect Menu item actions
    connect(ui->actionTransfer_Function_Editor,&QAction::triggered,
            this, &PBVRGUI::onTransfer_Function_EditorMenuAction);
    connect(ui->actionFilter_Information,&QAction::triggered,
            this, &PBVRGUI::onFilter_InformationMenuAction);
    connect(ui->actionConnect_To_Server,&QAction::triggered,
            this, &PBVRGUI::onConnect_To_ServerMenuAction);
    connect(ui->actionDisconnect_From_Server,&QAction::triggered,
            this, &PBVRGUI::onDisconnect_From_ServerMenuAction);
    connect(ui->actionExport,&QAction::triggered,
            this, &PBVRGUI::onExportMenuAction);
    connect(ui->actionAbout_PBVR_Client,&QAction::triggered,
            this, &PBVRGUI::onAbout_PBVR_ClientMenuAction);
    connect(ui->actionImport,&QAction::triggered,
            this, &PBVRGUI::onImportMenuAction);
    connect(ui->actionImport_Transfer_Functions,&QAction::triggered,
            this, &PBVRGUI::onImport_Transfer_FunctionsMenuAction);
    connect(ui->actionExport_Transfer_Functions,&QAction::triggered,
            this, &PBVRGUI::onExport_Transfer_FunctionsMenuAction);
    connect(ui->actionSystem_Status,&QAction::triggered,
            this, &PBVRGUI::onSystem_StatusMenuAction);
    connect(ui->actionRender_Options,&QAction::triggered,
            this, &PBVRGUI::onRender_OptionsMenuAction);
    connect(ui->actionVolume_Transform,&QAction::triggered,
            this, &PBVRGUI::onVolume_TransformMenuAction);
    connect(ui->actionTime_Controls,&QAction::triggered,
            this, &PBVRGUI::onTime_ControlsMenuAction);
    connect(ui->actionLegend_Options,&QAction::triggered,
            this, &PBVRGUI::onLegend_OptionsMenuAction);
    connect(ui->actionParticle_Panel,&QAction::triggered,
            this, &PBVRGUI::onParticle_PanelMenuAction);
    connect(ui->actionCoordinates,&QAction::triggered,
            this, &PBVRGUI::onCoordinatesMenuAction);
    connect(ui->actionPBVR_Client_Manual,&QAction::triggered,
            this, &PBVRGUI::onPBVR_Client_ManualMenuAction);
    connect(ui->actionRevert_to_Default_Layout,&QAction::triggered,
            this, &PBVRGUI::onRevert_to_Default_LayoutMenuAction);
    connect(ui->actionAnimation_Control,&QAction::triggered,
            this, &PBVRGUI::onAnimation_ControlsMenuAction);
    connect(ui->actionViewer_Controls,&QAction::triggered,
            this, &PBVRGUI::onViewer_ControlsMenuAction);
    connect(ui->actionZoomIn,&QAction::triggered,
            this,&PBVRGUI::on_actionZoomIn_triggered);
    connect(ui->actionZoomOut,&QAction::triggered,
            this,&PBVRGUI::on_actionZoomOut_triggered);
    connect(ui->actionXpos,&QAction::triggered,
            this,&PBVRGUI::on_actionXpos_triggered);
    connect(ui->actionXneg,&QAction::triggered,
            this,&PBVRGUI::on_actionXneg_triggered);
    connect(ui->actionYpos,&QAction::triggered,
            this,&PBVRGUI::on_actionYpos_triggered);
    connect(ui->actionYneg,&QAction::triggered,
            this,&PBVRGUI::on_actionYneg_triggered);
//    connect(ui->actionCPU,&QAction::triggered,
//            this,&PBVRGUI::on_actionCPU_triggered);
//    connect(ui->actionGPU,&QAction::triggered,
//            this,&PBVRGUI::on_actionGPU_triggered);

    connect(ui->actionAddFrame,&QAction::triggered,
            this,&PBVRGUI::on_actionAddFrame_triggered);
    connect(ui->actionRemoveFrame,&QAction::triggered,
            this,&PBVRGUI::on_actionRemoveFrame_triggered);
    connect(ui->actionPlayFrames,&QAction::triggered,
            this,&PBVRGUI::on_actionPlayFrames_triggered);

//    ui->RenderModeToolBar->setVisible(false);
}



PBVRGUI::~PBVRGUI()
{
    // Closing this down, crashes mac osx
    extCommand->m_glut_timer->stop();
    extCommand->m_server.close(extCommand->m_parameter);
    //    extCommand->comthread.quit();
    extCommand->comthread.wait();
    delete ui;
}

/**
 * @brief PBVRGUI::setCommandInstance setup the command instance
 */
void PBVRGUI::start()
{

    qInfo(" PBVRGUI::start() checkpoint 1");
    particlePanel.updateUI2Cmd();
    qInfo(" PBVRGUI::start() checkpoint 2");
    load_parameter_file(extCommand->argument);
    qInfo(" PBVRGUI::start() checkpoint 3");
    //    extCommand->CallBackApply(0);
    qInfo(" PBVRGUI::start() checkpoint 4");
    timecontrolPanel.updateCmd2UI(extCommand);
    qInfo(" PBVRGUI::start() checkpoint 5");
    kvs_renderarea->updateCommandInfo(extCommand);
    qInfo(" PBVRGUI::start() checkpoint 6");

    extCommand->startDetailedAnalysis();
    qInfo(" PBVRGUI::start() checkpoint 7");
    kvs_renderarea->setupEventHandlers();
    qInfo(" PBVRGUI::start() checkpoint 8");

    systemstatusPanel.updateSystemStatus();
    qInfo(" PBVRGUI::start() checkpoint 9");
    filterinfoPanel.updateFilterInfo();
    qInfo(" PBVRGUI::start() checkpoint 10");
    renderoptionPanel.updateCmd2UI();
    qInfo(" PBVRGUI::start() checkpoint 11");
    setBackgroundColor(VizParameterFile::getBackgroundColor());




    extCommand->CallBackApply(0);
    showNormal();
}


/**
 * @brief PBVRClient::addDocks add the ui docks.
 * @return int status
 */
int PBVRGUI::restoreAllState()
{
    restoreDefaultLayout();
    gui_settings=new QSettings("JAEA_CCSE", "PBVR_Client");
//    gui_settings->clear();
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    restoreGeometry(gui_settings->value("mainwindow/geometry").toByteArray());
    restoreState(gui_settings->value("mainwindow/windowState").toByteArray());
    return 0;
}
/**
 * @brief PBVRGUI::restoreDockState restores the state of a single dock
 * @param dock
 */
void PBVRGUI::restoreDockState(QDockWidget* dock)
{
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea );
    bool restored=restoreDockWidget(dock);
    dockstate_restored=dockstate_restored||restored;
    if ((dockWidgetArea(dock) == Qt::NoDockWidgetArea )&! dock->isFloating()){
        dock->setVisible(false);
    }
}

/**
 * @brief PBVRGUI::restoreDefaultLayout
 *  restores the default window layout
 *
 */
void PBVRGUI::restoreDefaultLayout()
{
    coordinatePanel.close();
    filterinfoPanel.close();
    legendPanel.close();
    particlePanel.close();
    renderoptionPanel.close();
    systemstatusPanel.close();
    timecontrolPanel.close();
    transformPanel.close();

    animationControls.close();
    systemstatusPanel.setFloating(false);
    filterinfoPanel.setFloating(false);
    timecontrolPanel.setFloating(false);

    addDockWidget(systemstatusPanel.default_area, &systemstatusPanel);
    addDockWidget(  filterinfoPanel.default_area, &filterinfoPanel);
    addDockWidget( timecontrolPanel.default_area, &timecontrolPanel);
    systemstatusPanel.setVisible(true);
    filterinfoPanel.setVisible(true);
    timecontrolPanel.setVisible(true);
    ui->AnimationToolBar->setVisible(true);
//    ui->RenderModeToolBar->setVisible(true);
    ui->XformToolBar->setVisible(true);
}
/**
 * @brief PBVRClient::showMessage, show a simple modal message box
 * @param message
 */
void PBVRGUI::showStatusMessage(QString message, bool showMsgBox)
{
    statusBar()->showMessage("Status:"+message);
    statusBar()->setToolTip(statusBar()->toolTip()+"\n"+message);
    if (showMsgBox){
        QMessageBox msgBox;
        msgBox.setText(message);
        msgBox.exec();
    }

    QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    QThread::msleep(400);
}

/**
 * @brief PBVRClient::showDock, show a dock in the selected dock_area
 * @param dock
 * @param dock_area
 */
void PBVRGUI::showDock(QDockWidget* dw, Qt::DockWidgetArea dwa)
{
    if ( dw->isFloating())
    {
        //noqinfo// qinfo()<<"Showing" <<dw->objectName()<<"as floating";
    }
    else {
        dwa=dockWidgetArea(dw)==Qt::NoDockWidgetArea?dwa:dockWidgetArea(dw);
        addDockWidget(dwa,dw);
        //noqinfo// qinfo()<<"Showing" <<dw->objectName()<<"as dockwidget";
    }
    dw->setVisible(true);
}


/**
 * @brief PBVRGUI::load_parameter_file load a parameter file
 *        as specified by the argument structure.
 * @param argument
 */
void PBVRGUI::load_parameter_file(kvs::visclient::Argument& argument)
{
    // Import parameter file
    if ( argument.m_parameter_file != "" )
    {
        showStatusMessage( "Reading Parameter File" );
        VizParameterFile::ReadParamFile( argument.m_parameter_file.c_str(), 0 ); // Lots of side effects here
        legendPanel.importFile( argument.m_parameter_file.c_str() );
        showStatusMessage("Importing Parameter File");
        tf_editor.importFile( argument.m_parameter_file.c_str() );
    }
#ifdef IS_MODE
    else{
        tf_editor.importExtendTransferFunction( extCommand->m_parameter.m_parameter_extend_transfer_function );
    }
#endif
    // Import Transfer Function File
    if ( argument.m_transfer_function_file_flag )
    {
        showStatusMessage("Importing transfer function File");
        tf_editor.importFile( argument.m_transfer_function_file_name );
    }

    //NO NEED m_Particle_UI.createPanel( command.m_parameter.m_client_server_mode, &command.m_parameter.m_particle_merge_param );
    // 粒子の表示・非表示情報を内部変数に反映
    particlePanel.setParam( &(extCommand->m_parameter.m_particle_merge_param) );

    // Serverのstep範囲情報を得る
#ifdef CS_MODE
     int ret=extCommand->getServerParticleInfomation( &extCommand->m_parameter, &extCommand->m_result );

     //    int ret = extCommand->getServerParticleInfomation(  ); // unused parameter
    if (ret < 0) {
        showStatusMessage("Failed to getServerParticleInfomation. Is the server still alive?",true);
        exit(1);
        //        exit(1);		// add by @hira at 2016/12/01 : 接続エラーの場合終了する
    }
#endif
    tf_editor.setCommandInstance(extCommand);
    tf_editor.apply();

    // pin引数で指定されたファイルを全てチェックしてstepの範囲を得る
    kvs::visclient::ParticleMerger tmp_merger;
    std::cout << " PBVRGUI::load_parameter_file :: "<<std::endl;

    std::cout << " PBVRGUI::load_parameter_file command_q.m_parameter.m_min_server_time_step  "<<extCommand->m_parameter.m_min_server_time_step << std::endl;
    std::cout << " PBVRGUI::load_parameter_file command_q.m_parameter.m_max_server_time_step   "<<extCommand->m_parameter.m_max_server_time_step  << std::endl;
    tmp_merger.setParam( extCommand->m_parameter.m_particle_merge_param, extCommand->m_parameter.m_min_server_time_step, extCommand->m_parameter.m_max_server_time_step );

    size_t local_time_start = tmp_merger.getMergedInitialTimeStep();
    size_t local_time_end = tmp_merger.getMergedFinalTimeStep();
    //    local_time_start = command_q.m_parameter.m_min_server_time_step;
    //    local_time_end = command_q.m_parameter.m_max_server_time_step;
    std::cout << " PBVRGUI::load_parameter_file local_time_start  "<<local_time_start << std::endl;
    std::cout << " PBVRGUI::load_parameter_file local_time_end  "<<local_time_end << std::endl;
    extCommand->generateDetailedParticles();
    //    Win3vd();
    std::cout << " ### Win3vd() done " << std::endl;
    if ( argument.m_parameter_file != "" )
    {
        //        VizParameterFile::UpdateWin3v( argument.m_parameter_file.c_str() );
    }
    //#endif
}




/**
 * @brief PBVRGUI::closeEvent
 * @param event
 */
void PBVRGUI::closeEvent(QCloseEvent *event)
{
    gui_settings->setValue( "mainwindow/geometry", saveGeometry() );
    gui_settings->setValue( "mainwindow/windowState", saveState() );
#ifdef ENABLE_CROP
    crop_dialog.close();
#endif
    coordinatePanel.close();
    tf_editor.close();
    particlePanel.close();
    QMainWindow::closeEvent(event);
}

/**
 * @brief PBVRClient::on * triggered slots. (Event Handlers)
 */
void PBVRGUI::onTransfer_Function_EditorMenuAction()
{
    tf_editor.show();
    tf_editor.raise();
    tf_editor.setWindowState(Qt::WindowActive) ;
}

void PBVRGUI::onFilter_InformationMenuAction()
{
    showDock(&filterinfoPanel,Qt::RightDockWidgetArea);
}

void PBVRGUI::onConnect_To_ServerMenuAction()
{
    showStatusMessage( "Connecting to server.... \nNot implemented\n");
}

void PBVRGUI::onDisconnect_From_ServerMenuAction()
{
    showStatusMessage( "Disconnecting from server....\nNot implemented\n");
}

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

void PBVRGUI::onAbout_PBVR_ClientMenuAction()
{
#ifdef CS_MODE
    const char* server_mode="CS";
#else
    const char* server_mode="IS";
#endif
    QString message=QString("\n\n\nPBVR QtClient \n\nServer mode:%2 \n\nVersion: %4").arg( server_mode ,STR(PBVR_VERSION));
    QMessageBox::aboutQt(this,  tr("About Qt"));
    QMessageBox::about(this,tr("About PBVR"), tr(message.toStdString().c_str()));
}

void PBVRGUI::onImportMenuAction()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Parameter File"), ".", tr("Parameter Files (*.ini *.INI *.*)"));
    if (!fileName.isEmpty()){
        VizParameterFile::ConversionClassToFloat(kvs_renderarea->getPointObjectXform());
        VizParameterFile::ReadParamFile( fileName.toStdString().c_str());
        tf_editor.importFile( fileName.toStdString());
        coordinatePanel.setUISynthesizer();
        legendPanel.importFile(fileName.toStdString().c_str());
        legendPanel.set2UI();
        kvs::Xform xf = VizParameterFile::ConversionFloatToClass();
        kvs_renderarea->setPointObjectXform(xf);
        kvs_renderarea->redraw();
        extCommand->CallBackApply(0);
    }
}
void PBVRGUI::onExportMenuAction()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Current Settings to Parameter File"), ".", tr("Parameter Files (*.ini *.INI)"));
    if( fileName.right(4) != ".ini" && fileName.right(4) != ".INI" )
    {
        fileName += ".ini";
    }
    if (!fileName.isEmpty()){
        VizParameterFile::ConversionClassToFloat(kvs_renderarea->getPointObjectXform());
        VizParameterFile::WriteParamFile( fileName.toStdString().c_str() ); // APPEND BY)M.Tanaka 2015.03.03
        tf_editor.exportFile( fileName.toStdString(), true ); // APPEND Fj 2015.03.05
        legendPanel.exportFile( fileName.toStdString() );
    }
}

void PBVRGUI::onImport_Transfer_FunctionsMenuAction()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Transfer Function Settings File"), ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )")  );
    if (!fileName.isEmpty()){
        tf_editor.importFile(fileName.toStdString());
    }
}

void PBVRGUI::onExport_Transfer_FunctionsMenuAction()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Current Transfer Function Settings to Parameter File"),
                                         ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )")  );
    if (!fileName.isEmpty()){
        tf_editor.exportFile(fileName.toStdString(), false);
    }
}
void PBVRGUI::onSystem_StatusMenuAction()
{
    showDock(&systemstatusPanel,  Qt::RightDockWidgetArea);
}

void PBVRGUI::onRender_OptionsMenuAction()
{
    showDock(&renderoptionPanel,  Qt::RightDockWidgetArea);
}

void PBVRGUI::onVolume_TransformMenuAction()
{
    showDock( &transformPanel, Qt::RightDockWidgetArea );
}

void PBVRGUI::onTime_ControlsMenuAction()
{
    showDock( &timecontrolPanel, Qt::BottomDockWidgetArea );
}

void PBVRGUI::onLegend_OptionsMenuAction()
{
    showDock( &legendPanel, Qt::RightDockWidgetArea );
}



void PBVRGUI::onParticle_PanelMenuAction()
{
    particlePanel.setFloating(true);
    particlePanel.showNormal();
}

void PBVRGUI::onCoordinatesMenuAction()
{
    showDock( &coordinatePanel, Qt::RightDockWidgetArea );
}

void PBVRGUI::onPBVR_Client_ManualMenuAction()
{

    helpdialog.show();
    return;
}

/**
 * @brief PBVRClient::onRevert_to_Default_LayoutMenuAction
 */
void PBVRGUI::onRevert_to_Default_LayoutMenuAction()
{
    restoreDefaultLayout();
}

void PBVRGUI::onFull_ScreenMenuAction()
{
//    setWindowState((windowState() ^ Qt::WindowFullScreen));
    showFullScreen();
}

void PBVRGUI::onAnimation_ControlsMenuAction()
{
    qInfo("ANIMATION CONTROLS START");
    showDock(&animationControls,  Qt::RightDockWidgetArea);
}

void PBVRGUI::keyPressEvent(QKeyEvent* ke)
{

    qInfo()<<"@@@ keyPressEvent @@@@"<<ke->text();
    ui->kvs_renderarea->keyPressEvent(ke);
}

void PBVRGUI::setBackgroundColor(kvs::RGBColor c){
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.07.20
    extCommand->m_screen->scene()->background()->setColor(c);
    extCommand->m_screen->update();
    update();
}
void PBVRGUI::onViewer_ControlsMenuAction()
{
    qInfo("VIEWER CONTROLS");
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.07.20
    vc.selected_color=kvs_renderarea->scene()->background()->color();
    vc.setCurrentFont(kvs_renderarea->font());
    int result = vc.exec();
    if (result == QDialog::Accepted){
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        kvs_renderarea->scene()->background()->setColor(vc.selected_color);
        kvs_renderarea->setLabelFont(vc.getFontSelection());
    }
}

//    ADD BY)T.Osaki 2020.03.03
void PBVRGUI::mousePressEvent(QMouseEvent *event)
{
    QWidget::setFocus();
}

void PBVRGUI::on_actionZoomIn_triggered()
{
    std::cout<<" Zoom  in button"<<std::endl;
    //    kvs::Xform xf= kvs_renderarea->scene()->objectManager()->xform();
    kvs::Vec3 scale(1.25,1.25,1.25);
    kvs_renderarea->scene()->objectManager()->scale( scale);
    kvs_renderarea->update();
}

void PBVRGUI::on_actionZoomOut_triggered()
{
    std::cout<<" Zoom  Out button"<<std::endl;
    kvs::Vec3 scale(0.8,0.8,0.8);
    kvs_renderarea->scene()->objectManager()->scale(scale);
    kvs_renderarea->update();
}

void PBVRGUI::on_actionXpos_triggered()
{
    std::cout<<" X pos button"<<std::endl;
    kvs::Vec3 xlate(0.2,0.0,0.0);
    kvs::Xform xf= kvs_renderarea->scene()->objectManager()->xform();
    kvs_renderarea->scene()->objectManager()->translate(xf.scaling()* xlate);
    kvs_renderarea->update();
}

void PBVRGUI::on_actionYpos_triggered()
{
    std::cout<<"  Y pos  button"<<std::endl;
    kvs::Vec3 xlate(0.0,0.2,0.0);
    kvs::Xform xf= kvs_renderarea->scene()->objectManager()->xform();
    kvs_renderarea->scene()->objectManager()->translate(xf.scaling()* xlate);
    kvs_renderarea->update();
}
void PBVRGUI::on_actionXneg_triggered()
{
    std::cout<<" X neg  button"<<std::endl;
    //    kvs::Xform xf= kvs_renderarea->scene()->objectManager()->xform();
    kvs::Vec3 xlate(-0.2,0.0,0.0);
    kvs::Xform xf= kvs_renderarea->scene()->objectManager()->xform();
    kvs_renderarea->scene()->objectManager()->translate(xf.scaling()* xlate);
    kvs_renderarea->update();
}

void PBVRGUI::on_actionYneg_triggered()
{
    std::cout<<" Y neg  button"<<std::endl;
    kvs::Vec3 xlate(0.0,-0.2,0.0);
    kvs::Xform xf= kvs_renderarea->scene()->objectManager()->xform();
    kvs_renderarea->scene()->objectManager()->translate(xf.scaling()* xlate);
    kvs_renderarea->update();
}

//void PBVRGUI::on_actionCPU_triggered()
//{
//    ui->actionGPU->setChecked(false);
//    ui->actionGPU->setDisabled(false);
//    ui->actionCPU->setDisabled(true);
//    kvs_renderarea->switch_gpu(false);
//}

//void PBVRGUI::on_actionGPU_triggered()
//{
//    ui->actionCPU->setChecked(false);
//    ui->actionCPU->setDisabled(false);
//    ui->actionGPU->setDisabled(true);
//    kvs_renderarea->switch_gpu(true);
//}

void PBVRGUI::on_actionAddFrame_triggered()
{
    kvs_renderarea->animation_add();

}
void PBVRGUI::on_actionRemoveFrame_triggered()
{
    kvs_renderarea->animation_del();
}
void PBVRGUI::on_actionPlayFrames_triggered()
{
    kvs_renderarea->animation_play();
}

