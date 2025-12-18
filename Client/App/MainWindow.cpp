#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow( kvs::qt::Application& app, QWidget *parent )
    : QMainWindow( parent )
    , ui( new Ui::MainWindow )
    , m_screen( new kvs::qt::jaea::Screen( &app ) )
    , m_compositor( new kvs::StochasticRenderingCompositor( m_screen->scene() ) )
    , m_color_map_bar( nullptr )
    , m_orientation_axis( nullptr )
    , m_fps_label( nullptr )
    , m_time_step_label( nullptr )
    , m_initialize_camera_xform( kvs::Mat4(
          1, 0, 0, 0 ,
          0, 1, 0, 0 ,
          0, 0, 1, 12,
          0, 0, 0, 1
          ) )
    , m_web_sockets( new WebSocketPair )
    , m_viz_mode( new Viz::Mode( Viz::Mode::Local ) )
    // ToolBar
    , m_color_map_bar_selector_tool_bar( new ColorMapSelectorToolBar( m_screen, this ) ) // FIXME:未実装
    , m_play_back_control_tool_bar     ( new PlayBackControlToolBar( m_web_sockets, this ) )
    , m_time_step_control_tool_bar     ( new TimeStepControlToolBar( m_web_sockets, this ) )
    , m_total_particles_tool_bar       ( new TotalParticlesToolBar( this ) ) // FIXME:未実装
    // Widget
    , m_animation_control              ( new AnimationControl( m_screen, this ) )
    , m_communication                  ( new Communication( m_screen, m_web_sockets, m_viz_mode, this ) )
    , m_glyph_editor                   ( new GlyphEditor( m_web_sockets, this ) )
    , m_object_editor                  ( new ObjectEditorWIP( m_web_sockets, m_viz_mode, m_screen, this ) )
    , m_plot_over_line_editor          ( new PlotOverLineEditor( m_screen, m_web_sockets, this ) )
    , m_point_size_control             ( new PointSizeControl( m_screen, this ) )
    , m_preference                     ( new Preference( this ) )
    , m_repetition_level_control       ( new RepetitionLevelControl( m_screen, m_compositor, this ) )
    , m_shading_control                ( new ShadingControl( m_screen, this ) )
    , m_transfer_function_editor       ( new TransferFunctionEditorWIP( m_web_sockets, this ) )
    , m_volume_transform               ( new VolumeTransform( m_screen, this ) )
{
    ui->setupUi( this );
    setWindowTitle( QString( "pbvr_client - %1" ).arg( GIT_BRANCH_NAME ) );

    // NOTE:コンポジターの初期リピートレベルを設定
    m_compositor->setRepetitionLevel( DefaultRepetitionLevel );
    m_screen->setEvent( m_compositor );

    // NOTE:スクリーンの初期サイズを設定、レイアウト管理下に置く
    m_screen->setFixedSize( DefaultScreenSize, DefaultScreenSize );
    ui->screenArea->addWidget( m_screen );

    m_color_map_bar    = new kvs::ColorMapBar( m_screen );
    m_orientation_axis = new kvs::OrientationAxis( m_screen, m_screen->scene() );
    m_fps_label        = new kvs::Label( m_screen );
    m_time_step_label  = new kvs::Label( m_screen );

    { // NOTE:スクリーンのシグナルスロット接続
        connect( m_screen , &kvs::qt::jaea::Screen::addKeyFrameAdd    , m_animation_control, &AnimationControl::onAddKeyFrameAdd );
        connect( m_screen , &kvs::qt::jaea::Screen::removeLastKeyFrame, m_animation_control, &AnimationControl::onRemoveLastKeyFrame );
        connect( m_screen , &kvs::qt::jaea::Screen::clearKeyFrame     , m_animation_control, &AnimationControl::onClearKeyFrame );
        connect( m_screen , &kvs::qt::jaea::Screen::playKeyFrame      , m_animation_control, &AnimationControl::onPlayKeyFrame );
        connect( m_screen , &kvs::qt::jaea::Screen::loadKeyFrameFile  , m_animation_control, &AnimationControl::onLoadKeyFrameFile );
        connect( m_screen , &kvs::qt::jaea::Screen::saveKeyFrameFile  , m_animation_control, &AnimationControl::onSaveKeyFrameFile );

        connect( m_screen , &kvs::qt::jaea::Screen::updateTranslation , m_plot_over_line_editor, &PlotOverLineEditor::onUpdateTranslation );
    }

    {
        connect( this, &MainWindow::readyScreen                 , m_preference              , &Preference::onReadyScreen );
        connect( this, &MainWindow::updateInitialRepetitionLevel, m_repetition_level_control, &RepetitionLevelControl::onUpdateCurrentRepetitionLevel );
        { // FIXME:KPI
            m_load_action = new QAction( tr( "Load" ), this );
            connect( m_load_action, &QAction::triggered, this, &MainWindow::onLoad );
            ui->menuPBVRClient->addAction( m_load_action );

            m_save_action = new QAction( tr( "Save" ), this );
            connect( m_save_action, &QAction::triggered, this, &MainWindow::onSave );
            ui->menuPBVRClient->addAction( m_save_action );

            connect(
                this,
                &MainWindow::load,
                this,
                [this]( const QString& filePath )
                {
                    m_time_step_control_tool_bar     ->onLoadParameter( filePath );
                    m_color_map_bar_selector_tool_bar->onLoadParameter( filePath );
                    m_animation_control              ->onLoadParameter( filePath );
                    m_communication                  ->onLoadParameter( filePath );
                    m_glyph_editor                   ->onLoadParameter( filePath );
                    m_object_editor                  ->loadParameter( filePath );
                    m_plot_over_line_editor          ->onLoadParameter( filePath );
                    m_point_size_control             ->onLoadParameter( filePath );
                    m_repetition_level_control       ->onLoadParameter( filePath );
                    m_shading_control                ->onLoadParameter( filePath );
                    m_transfer_function_editor       ->loadParameter( filePath );
                    m_volume_transform               ->onLoadParameter( filePath );
                }
                );
            connect(
                this,
                &MainWindow::save,
                this,
                [this]( const QString& filePath )
                {
                    m_time_step_control_tool_bar     ->onSaveParameter( filePath );
                    m_color_map_bar_selector_tool_bar->onSaveParameter( filePath );
                    m_animation_control              ->onSaveParameter( filePath );
                    m_communication                  ->onSaveParameter( filePath );
                    m_glyph_editor                   ->onSaveParameter( filePath );
                    m_object_editor                  ->saveParameter( filePath );
                    m_plot_over_line_editor          ->onSaveParameter( filePath );
                    m_point_size_control             ->onSaveParameter( filePath );
                    m_repetition_level_control       ->onSaveParameter( filePath );
                    m_shading_control                ->onSaveParameter( filePath );
                    m_transfer_function_editor       ->saveParameter( filePath );
                    m_volume_transform               ->onSaveParameter( filePath );
                }
                );
        }
    }

    toolBarInitialize();
    animationControlInitialize();
    communicationInitialize();
    glyphEditorInitialize();
    objectEditorInitialize();
    plotOverLineEditorInitialize();
    pointSizeControlInitialize();
    preferenceInitialize();
    repetitionLevelControlInitialize();
    shadingControlInitialize();
    transferFunctionEditorInitialize();
    volumeTransformInitialize();

#ifdef OPENXR_SCREEN
    m_vr_listener = new VRHandControllerListener( m_screen );
    m_screen->addEvent( m_vr_listener );
    // connect( m_vr_listener, &VRHandControllerListener::drawPlotOverLine, m_plot_over_line, &PlotOverLine::drawPlotOverLine );
    // connect( m_vr_listener, &VRHandControllerListener::showHidePlotOverLine, m_plot_over_line, &PlotOverLine::showHidePlotOverLine );
    connect( m_vr_listener, &VRHandControllerListener::sendVRSharePoint, m_communication, &Communication::onVRSharePoint );
#endif
    if( m_animation_control && m_communication )
    {
        tabifyDockWidget( m_animation_control, m_communication );
    }
    this->show();

    emit readyScreen();
    initializeAfterShow();
}

MainWindow::~MainWindow()
{
    delete m_web_sockets;
    delete m_time_step_label;
    delete m_fps_label;
    delete m_orientation_axis;
    delete m_color_map_bar;
    delete m_compositor;
    delete m_screen;
    delete ui;
}

void MainWindow::onUpdateStatusBarMessage( const QString& message )
{
    ui->statusBar->showMessage( message, 5000 ); // 5秒間表示
}

void MainWindow::toolBarInitialize()
{
    if( m_time_step_control_tool_bar )
    {
        connect( m_time_step_control_tool_bar, &TimeStepControlToolBar::requestDataAt        , m_object_editor             , &ObjectEditorWIP::onRequestDataAt );
        connect( m_time_step_control_tool_bar, &TimeStepControlToolBar::dataRequestCompleted , m_play_back_control_tool_bar, &PlayBackControlToolBar::onDataRequestCompleted );

        this->addToolBar( Qt::TopToolBarArea, m_time_step_control_tool_bar );
        this->addToolBarBreak( Qt::TopToolBarArea );
    }

    if( m_total_particles_tool_bar )
    {
        this->addToolBar( Qt::TopToolBarArea, m_total_particles_tool_bar );
    }

    if( m_color_map_bar_selector_tool_bar )
    {
        m_color_map_bar_selector_tool_bar->setColorMapBar( m_color_map_bar );

        this->addToolBar( Qt::TopToolBarArea, m_color_map_bar_selector_tool_bar );
        this->addToolBarBreak( Qt::TopToolBarArea );
    }

    if( m_play_back_control_tool_bar )
    {
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::first   , m_time_step_control_tool_bar, &TimeStepControlToolBar::onFirst );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::previous, m_time_step_control_tool_bar, &TimeStepControlToolBar::onPrevious );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::reverse , m_time_step_control_tool_bar, &TimeStepControlToolBar::onReverse );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::play    , m_time_step_control_tool_bar, &TimeStepControlToolBar::onPlay );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::next    , m_time_step_control_tool_bar, &TimeStepControlToolBar::onNext );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::last    , m_time_step_control_tool_bar, &TimeStepControlToolBar::onLast );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::keepLast, m_time_step_control_tool_bar, &TimeStepControlToolBar::onKeepLast );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::jump    , m_time_step_control_tool_bar, &TimeStepControlToolBar::onJump );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::loop    , m_time_step_control_tool_bar, &TimeStepControlToolBar::onLoop );

        this->addToolBar( Qt::TopToolBarArea, m_play_back_control_tool_bar );
    }
}

void MainWindow::animationControlInitialize()
{
    if( m_animation_control )
    {
        m_animation_control_action = new QAction( tr( "Animation Control" ), this );
        ui->menuTools->addAction( m_animation_control_action );

        m_animation_control->adjustSize();
        m_animation_control->close();
        addDockWidget( Qt::RightDockWidgetArea, m_animation_control );

        connect( m_animation_control_action, &QAction::triggered, this, &MainWindow::onAnimationControl );
    }
}

void MainWindow::communicationInitialize()
{
    if( m_communication )
    {
        m_communication_action = new QAction( tr( "Communication" ), this );

        // NOTE:Toolsメニューの先頭に挿入
        if( !ui->menuTools->actions().isEmpty()  ) ui->menuTools->insertAction( ui->menuTools->actions().first(), m_communication_action );
        else                                       ui->menuTools->addAction( m_communication_action );
        ui->menuTools->addSeparator();

        m_communication->adjustSize();
        addDockWidget( Qt::RightDockWidgetArea, m_communication );
        // FIXME:接続が汚すぎる
        connect( m_communication, &Communication::updateStatusBarMessage          , this                        , &MainWindow::onUpdateStatusBarMessage );
        connect( m_communication, &Communication::updateServerState               , this                        , &MainWindow::onUpdateServerState );

        connect( m_communication, &Communication::receiveTimeStepControlParameter , m_time_step_control_tool_bar, &TimeStepControlToolBar::onReceiveTimeStepControlParameter );
        connect( m_communication, &Communication::updateOperatorState             , m_time_step_control_tool_bar, &TimeStepControlToolBar::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState             , m_play_back_control_tool_bar, &PlayBackControlToolBar::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState             , m_glyph_editor              , &GlyphEditor::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState             , m_object_editor             , &ObjectEditorWIP::updateOperatorState );
        connect( m_communication, &Communication::updateOperatorState             , m_plot_over_line_editor     , &PlotOverLineEditor::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState             , m_transfer_function_editor  , &TransferFunctionEditorWIP::updateOperatorState );
        connect( m_communication, &Communication::receiveTransferFunctionParameter, m_transfer_function_editor  , &TransferFunctionEditorWIP::onReceiveTransferFunctionParameter );

        connect( m_communication, &Communication::receiveGlyphParameter           , m_glyph_editor              , &GlyphEditor::onReceiveGlyphParameter );

        connect( m_communication, &Communication::unpack                          , m_object_editor             , &ObjectEditorWIP::unpack );
        connect( m_communication, &Communication::addObjectToModel                , m_object_editor             , &ObjectEditorWIP::addObjectToModel );
        connect( m_communication, &Communication::receiveObjectInfoParameter      , m_object_editor             , &ObjectEditorWIP::onReceiveObjectInfoParameter );

        connect( m_communication, &Communication::receivePlotOverLineParameter    , m_plot_over_line_editor     , &PlotOverLineEditor::onReceivePlotOverLineParameter );

        connect( m_communication_action, &QAction::triggered, this, &MainWindow::onCommunication );
    }
}

void MainWindow::glyphEditorInitialize()
{
    if( m_glyph_editor )
    {
        m_glyph_editor_action = new QAction( tr( "Glyph Editor" ), this );
        ui->menuTools->addAction( m_glyph_editor_action );
        m_glyph_editor_action->setEnabled( false ); // NOTE:サーバ接続前は無効

        connect( m_glyph_editor_action, &QAction::triggered, this, &MainWindow::onGlyphEditor );

        // m_glyph_editor_wip->onUpdateNumberOfVector( 3 ); // DEBUG:成分数に応じてUIが変化するか確認
    }
}

void MainWindow::objectEditorInitialize()
{
    if( m_object_editor )
    {
        m_object_editor_action = new QAction( tr( "Object Editor" ), this );
        ui->menuTools->addAction( m_object_editor_action );
        m_object_editor->adjustSize();
        addDockWidget( Qt::LeftDockWidgetArea, m_object_editor );

        connect( m_object_editor, &ObjectEditorWIP::updateTotalTimeStepRange, m_time_step_control_tool_bar, &TimeStepControlToolBar::updateTotalTimeStepRange );
        connect( m_object_editor, &ObjectEditorWIP::updateFocus             , m_plot_over_line_editor     , &PlotOverLineEditor::onUpdateFocus );
        connect( m_object_editor, &ObjectEditorWIP::updateTranslation       , m_plot_over_line_editor     , &PlotOverLineEditor::onUpdateTranslation );
        connect( m_object_editor, &ObjectEditorWIP::shading                 , m_shading_control           , &ShadingControl::onShading );
        connect( m_object_editor, &ObjectEditorWIP::dataRequestCompleted    , m_time_step_control_tool_bar, &TimeStepControlToolBar::onDataRequestCompleted );

        connect( m_object_editor_action, &QAction::triggered, this, &MainWindow::onObjectEditor );
    }
}

void MainWindow::plotOverLineEditorInitialize()
{
    if( m_plot_over_line_editor )
    {
        m_plot_over_line_editor_action = new QAction( tr( "Plot Over Line Editor" ), this );
        ui->menuTools->addAction( m_plot_over_line_editor_action );
        m_plot_over_line_editor_action->setEnabled( false ); // サーバ接続前は無効
        m_plot_over_line_editor->adjustSize();
        addDockWidget( Qt::LeftDockWidgetArea, m_plot_over_line_editor );
        m_plot_over_line_editor->close();

        connect( m_plot_over_line_editor_action, &QAction::triggered, this, &MainWindow::onPlotOverLineEditor );

        // m_plot_over_line_editor_wip->updateNumberOfVector( 3 ); // DEBUG:成分数に応じてUIが変化するか確認
    }
}

void MainWindow::pointSizeControlInitialize()
{
    if( m_point_size_control )
    {
        m_point_size_control_action = new QAction( tr( "Point Size Control" ), this );
        ui->menuTools->addAction( m_point_size_control_action );
        m_point_size_control->adjustSize();
        m_point_size_control->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_point_size_control );

        connect( m_point_size_control_action, &QAction::triggered, this, &MainWindow::onPointSizeControl );
    }
}

void MainWindow::preferenceInitialize()
{
    if( m_preference )
    {
        m_preference_action = new QAction( tr( "Preference" ), this );
        ui->menuPBVRClient->addAction( m_preference_action );
        m_preference->adjustSize();

        m_preference->setScreen( m_screen );
        m_preference->setCompositor( m_compositor );
        m_preference->setColorMapBar( m_color_map_bar );
        m_preference->setOrientationAxis( m_orientation_axis );
        m_preference->setFpsLabel( m_fps_label );
        m_preference->setTimeStepLabel( m_time_step_label );

        connect( m_preference_action, &QAction::triggered, this, &MainWindow::onPreference );
    }
}

void MainWindow::repetitionLevelControlInitialize()
{
    if( m_repetition_level_control )
    {
        m_repetition_level_control_action = new QAction( tr( "Repetition Level Control" ), this );
        ui->menuTools->addAction( m_repetition_level_control_action );
        m_repetition_level_control->adjustSize();
        m_repetition_level_control->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_repetition_level_control );

        connect( m_repetition_level_control       , &RepetitionLevelControl::shading, m_shading_control, &ShadingControl::onShading );
        connect( m_repetition_level_control_action, &QAction::triggered             , this             , &MainWindow::onRepetitionLevelControl );
        emit updateInitialRepetitionLevel();
    }
}

void MainWindow::shadingControlInitialize()
{
    if( m_shading_control )
    {
        m_shading_control_action = new QAction( tr( "Shading Control" ), this );
        ui->menuTools->addAction( m_shading_control_action );
        m_shading_control->adjustSize();
        m_shading_control->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_shading_control );

        connect( m_shading_control_action, &QAction::triggered, this, &MainWindow::onShadingControl );
    }
}

void MainWindow::transferFunctionEditorInitialize()
{
    if( m_transfer_function_editor )
    {
        m_transfer_function_editor_action = new QAction( tr( "Transfer Function Editor"), this );
        ui->menuTools->addAction( m_transfer_function_editor_action );
        m_transfer_function_editor_action->setEnabled( false ); // サーバ接続前は無効
        m_transfer_function_editor->adjustSize();

        connect( m_transfer_function_editor_action, &QAction::triggered, this, &MainWindow::onTransferFunctionEditor );
        // connect( m_transfer_function_editor         , &TransferFunctionEditor::updateColorMapBar                    , m_color_map_bar_selector_tool_bar , &ColorMapSelectorToolBar::updateColorMapBar );
        // connect( m_transfer_function_editor         , &TransferFunctionEditor::failedTransferFunctionImport         , m_connect                         , &Connect::failedTransferFunctionImport );
        // connect( m_transfer_function_editor         , &TransferFunctionEditor::successTransferFunctionImport        , m_connect                         , &Connect::successTransferFunctionImport );
        // connect( m_transfer_function_editor         , &TransferFunctionEditor::updateTransferFunctionClientMessage  , m_connect                         , &Connect::updateTransferFunctionClientMessage );
        // connect( m_transfer_function_editor         , &TransferFunctionEditor::requestReplaceServerPointObject      , m_object_editor                   , &ObjectEditor::requestReplaceServerPointObject );
    }
}

void MainWindow::volumeTransformInitialize()
{
    if( m_volume_transform )
    {
        m_volume_transform_action = new QAction( tr( "Volume Transform" ), this );
        ui->menuTools->addAction( m_volume_transform_action );
        m_volume_transform->adjustSize();
        m_volume_transform->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_volume_transform );

        connect( m_volume_transform       , &VolumeTransform::updateTranslation , m_plot_over_line_editor, &PlotOverLineEditor::onUpdateTranslation );
        connect( m_volume_transform_action, &QAction::triggered, this, &MainWindow::onVolumeTransform );
    }
}

void MainWindow::initializeAfterShow()
{
    const QString applicationDirPath = QCoreApplication::applicationDirPath();

    const QString shaderDIR = QDir( applicationDirPath ).filePath( "Shader" );
    kvs::ShaderSource::AddSearchPath( shaderDIR.toUtf8().constData() );

    const QString fontDIR = QDir( applicationDirPath ).filePath("Font/");

    struct FontInfo
    {
        const char* name;
        const char* file;
    };

    FontInfo fonts[] = {
        { "SansRegular",      "NotoSans-Regular.ttf" },
        { "SansItalic",       "NotoSans-Italic.ttf" },
        { "SansBold",         "NotoSans-Bold.ttf" },
        { "SansBoldItalic",   "NotoSans-BoldItalic.ttf" },
        { "SerifRegular",     "NotoSerif-Regular.ttf" },
        { "SerifItalic",      "NotoSerif-Italic.ttf" },
        { "SerifBold",        "NotoSerif-Bold.ttf" },
        { "SerifBoldItalic",  "NotoSerif-BoldItalic.ttf" },
        { "Icon",             "entypo.ttf" }
    };

    for( const auto& f : fonts )
    {
        QString fullPath = fontDIR + f.file;
        m_screen->paintDevice()->textEngine()->addFont( f.name, fullPath.toUtf8().constData() );
    }
}

void MainWindow::onUpdateServerState( bool serverState ) // true:接続中
{
    if( m_play_back_control_tool_bar )
    {
        m_play_back_control_tool_bar->onReset();
    }

    if( m_glyph_editor && m_glyph_editor_action )
    {
        // TODO:サーバーと導通時にサーバから成分数を送ってもらう必要がある。 成分数が3未満の場合、GlyphEditorは開けなくする必要がある。
        m_glyph_editor_action->setEnabled( serverState );
        if( !serverState )
        {
            m_glyph_editor->close();
            m_glyph_editor->onReset();
        }
    }

    if( m_object_editor && m_object_editor_action )
    {
        /*
         * NOTE:
         * ローカルモード(サーバと接続せずにローカルデータのみ閲覧するモード)
         * スタンドアロンモード(クライアントとサーバを同じマシンで起動、接続するモード)
         * クラサバモード(クライアントとサーバを別マシンで起動、接続するモード)
         * In-situモード(クライアントとサーバを別マシンで起動、接続するモード)
         * 上記の対応のため接続/切断時にObjectEditorをリセットする必要があります。
         */
        m_object_editor->reset();
    }

    if( m_plot_over_line_editor && m_plot_over_line_editor_action )
    {
        m_plot_over_line_editor_action->setEnabled( serverState );
        if( !serverState )
        {
            m_plot_over_line_editor->close();
            m_plot_over_line_editor->onReset();
        }
    }

    if( m_transfer_function_editor && m_transfer_function_editor_action )
    {
        m_transfer_function_editor_action->setEnabled( serverState );
        if( !serverState )
        {
            m_transfer_function_editor->close();
            m_transfer_function_editor->reset();
        }
    }
}

void MainWindow::onLoad()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr( "Load File" ), QString(), tr( "JSON Files (*.json);;All Files (*)" ) );
    emit load( fileName );
}

void MainWindow::onSave()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr( "Load File" ), QString(), tr( "JSON Files (*.json);;All Files (*)" ) );
    emit save( fileName );
}
