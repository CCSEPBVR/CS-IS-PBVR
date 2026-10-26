#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QPushButton>
#include <kvs/EventTimer>
#include <kvs/PointImporter>
#include <kvs/PolygonImporter>
#include <kvs/StochasticPolygonRenderer>

MainWindow::MainWindow( kvs::qt::Application& app, QWidget *parent )
    : QMainWindow( parent )
    , ui                ( new Ui::MainWindow )
    , m_screen          ( new kvs::qt::jaea::Screen( &app ) )
    , m_compositor      ( new kvs::StochasticRenderingCompositor( m_screen->scene() ) )
    , m_color_map_bar   ( nullptr )
    , m_orientation_axis( nullptr )
    , m_fps_label       ( nullptr )
    , m_time_step_label ( nullptr )
    , m_web_sockets     ( new WebSocketPair )
    , m_viz_mode        ( new Viz::Mode( Viz::Mode::Local ) )
    // ToolBar
    , m_color_map_bar_selector_tool_bar( new ColorMapSelectorToolBar( m_screen, this ) )
    , m_play_back_control_tool_bar     ( new PlayBackControlToolBar( m_web_sockets, this ) )
    , m_time_step_control_tool_bar     ( new TimeStepControlToolBar( m_web_sockets, this ) )
    , m_total_particles_tool_bar       ( new TotalParticlesToolBar( m_screen, this ) )
    // Widget
    , m_animation_control              ( new AnimationControl( m_screen, this ) )
    , m_communication                  ( new Communication( m_screen, m_web_sockets, m_viz_mode, this ) )
    , m_glyph_editor                   ( new GlyphEditor( m_web_sockets, this ) )
    , m_object_editor                  ( new ObjectEditor( m_screen, m_web_sockets, m_viz_mode, this ) )
    , m_plot_over_line_editor          ( new PlotOverLineEditor( m_screen, m_web_sockets, this ) )
    , m_plot_over_time_editor          ( new PlotOverTimeEditor( m_screen, m_web_sockets, this ) )
    , m_point_size_control             ( new PointSizeControl( m_screen, this ) )
    , m_preference                     ( new Preference( this ) )
    , m_repetition_level_control       ( new RepetitionLevelControl( m_screen, m_compositor, m_web_sockets, m_viz_mode, this ) )
    , m_shading_control                ( new ShadingControl( m_screen, this ) )
    , m_ensemble_transfer_function_editor( new EnsembleTransferFunctionEditor( m_web_sockets, this ) )
    , m_transfer_function_editor       ( new TransferFunctionEditor( m_web_sockets, this ) )
    , m_volume_transform               ( new VolumeTransform( m_screen, this ) )
{
    ui->setupUi( this );
    setWindowTitle( QString( "pbvr_client - %1" ).arg( GIT_BRANCH_NAME ) );

    // NOTE:コンポジターの初期リピートレベルを設定
    m_compositor->setRepetitionLevel( k_default_repetition_level );
    m_screen->setEvent( m_compositor );

    // NOTE:スクリーンの初期サイズを設定、レイアウト管理下に置く( w, h )
    m_screen->setFixedSize( k_default_screen_resolution, k_default_screen_resolution );
    ui->screenArea->addWidget( m_screen );

    m_color_map_bar    = new kvs::ColorMapBar( m_screen );
    m_orientation_axis = new kvs::OrientationAxis( m_screen, m_screen->scene() );
    m_fps_label        = new kvs::Label( m_screen );
    m_time_step_label  = new kvs::Label( m_screen );

    // NOTE:Screenのシグナルスロット接続
    {
        connect( m_screen, &kvs::qt::jaea::Screen::addKeyFrameAdd    , m_animation_control    , &AnimationControl::onAddKeyFrameAdd );
        connect( m_screen, &kvs::qt::jaea::Screen::removeLastKeyFrame, m_animation_control    , &AnimationControl::onRemoveLastKeyFrame );
        connect( m_screen, &kvs::qt::jaea::Screen::clearKeyFrame     , m_animation_control    , &AnimationControl::onClearKeyFrame );
        connect( m_screen, &kvs::qt::jaea::Screen::playKeyFrame      , m_animation_control    , &AnimationControl::onPlayKeyFrame );
        connect( m_screen, &kvs::qt::jaea::Screen::loadKeyFrameFile  , m_animation_control    , &AnimationControl::onLoadKeyFrameFile );
        connect( m_screen, &kvs::qt::jaea::Screen::saveKeyFrameFile  , m_animation_control    , &AnimationControl::onSaveKeyFrameFile );
        connect( m_screen, &kvs::qt::jaea::Screen::updateTranslation , m_plot_over_line_editor, &PlotOverLineEditor::onUpdateTranslation );
    }

    // NOTE:MainWindowのシグナルスロット接続
    {
        connect( this, &MainWindow::screenInitialized           , m_preference              , &Preference::onScreenInitialized );
        connect( this, &MainWindow::updateInitialRepetitionLevel, m_repetition_level_control, &RepetitionLevelControl::onUpdateCurrentRepetitionLevel );

        m_load_action = new QAction( tr( "Load" ), this );
        connect( m_load_action, &QAction::triggered, this, &MainWindow::onLoad );
        ui->menuPBVRClient->addAction( m_load_action );

        m_save_action = new QAction( tr( "Save" ), this );
        connect( m_save_action, &QAction::triggered, this, &MainWindow::onSave );
        ui->menuPBVRClient->addAction( m_save_action );

        // KPI
        connect(
            this,
            &MainWindow::load,
            this,
            [this]( const QString& filePath )
            {
                if( m_time_step_control_tool_bar      ) m_time_step_control_tool_bar     ->onLoadParameter( filePath );
                if( m_color_map_bar_selector_tool_bar ) m_color_map_bar_selector_tool_bar->onLoadParameter( filePath );
                if( m_animation_control               ) m_animation_control              ->onLoadParameter( filePath );
                if( m_communication                   ) m_communication                  ->onLoadParameter( filePath );
                if( m_glyph_editor                    ) m_glyph_editor                   ->onLoadParameter( filePath );
                if( m_object_editor                   ) m_object_editor                  ->onLoadParameter( filePath );
                if( m_plot_over_line_editor           ) m_plot_over_line_editor          ->onLoadParameter( filePath );
                if( m_plot_over_time_editor           ) m_plot_over_time_editor          ->onLoadParameter( filePath );
                if( m_point_size_control              ) m_point_size_control             ->onLoadParameter( filePath );
                if( m_repetition_level_control        ) m_repetition_level_control       ->onLoadParameter( filePath );
                if( m_shading_control                 ) m_shading_control                ->onLoadParameter( filePath );
                if( m_transfer_function_editor        ) m_transfer_function_editor       ->onLoadParameter( filePath );
                if( m_volume_transform                ) m_volume_transform               ->onLoadParameter( filePath );
            }
            );
        connect(
            this,
            &MainWindow::save,
            this,
            [this]( const QString& filePath )
            {
                if( m_time_step_control_tool_bar      ) m_time_step_control_tool_bar     ->onSaveParameter( filePath );
                if( m_color_map_bar_selector_tool_bar ) m_color_map_bar_selector_tool_bar->onSaveParameter( filePath );
                if( m_animation_control               ) m_animation_control              ->onSaveParameter( filePath );
                if( m_communication                   ) m_communication                  ->onSaveParameter( filePath );
                if( m_glyph_editor                    ) m_glyph_editor                   ->onSaveParameter( filePath );
                if( m_object_editor                   ) m_object_editor                  ->onSaveParameter( filePath );
                if( m_plot_over_line_editor           ) m_plot_over_line_editor          ->onSaveParameter( filePath );
                if( m_plot_over_time_editor           ) m_plot_over_time_editor          ->onSaveParameter( filePath );
                if( m_point_size_control              ) m_point_size_control             ->onSaveParameter( filePath );
                if( m_repetition_level_control        ) m_repetition_level_control       ->onSaveParameter( filePath );
                if( m_shading_control                 ) m_shading_control                ->onSaveParameter( filePath );
                if( m_transfer_function_editor        ) m_transfer_function_editor       ->onSaveParameter( filePath );
                if( m_volume_transform                ) m_volume_transform               ->onSaveParameter( filePath );
            }
            );
    }

    // NOTE:ToolBarの初期化
    initializeToolBar();
    initializeMenuBar();

    // NOTE:Action & Widgetの初期化
    initializeAnimationControl();
    initializeCommunication();
    initializeGlyphEditor();
    initializeObjectEditor();
    initializePlotOverLineEditor();
    initializePlotOverTimeEditor();
    initializePointSizeControl();
    initializePreference();
    initializeRepetitionLevelControl();
    initializeShadingControl();
    initializeEnsembleTransferFunctionEditor();
    initializeTransferFunctionEditor();
    initializeVolumeTransform();

    this->show();

    // NOTE:描画リソース(Shader/Font)の初期化
    initializeShaderAndFonts();

    emit screenInitialized();

#ifdef OPENXR_SCREEN
    m_vr_listener = new VRHandControllerListener( m_screen );
    m_screen->addEvent( m_vr_listener );
    connect( m_vr_listener, &VRHandControllerListener::drawVRPlotOverLine          , m_plot_over_line_editor, &PlotOverLineEditor::onDrawVRPlotOverLine );
    connect( m_vr_listener, &VRHandControllerListener::toggleShowHideVRPlotOverLine, m_plot_over_line_editor, &PlotOverLineEditor::onToggleShowHideVRPlotOverLine );

    connect( m_vr_listener, &VRHandControllerListener::drawVRPlotOverTime          , m_plot_over_time_editor, &PlotOverTimeEditor::onDrawVRPlotOverTime );
    connect( m_vr_listener, &VRHandControllerListener::toggleShowHideVRPlotOverTime, m_plot_over_time_editor, &PlotOverTimeEditor::onToggleShowHideVRPlotOverTime );
    
    connect( m_vr_listener, &VRHandControllerListener::toggleShowHideSharePoint    , m_communication        , &Communication::onToggleShowHideSharePoint );
    connect( m_vr_listener, &VRHandControllerListener::drawVRSharePoint            , m_communication        , &Communication::onDrawVRSharePoint );
#endif

    std::vector<kvs::Real32> polygonCoords =
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f
        };

    std::vector<kvs::UInt32> polygonConnections =
        {
            0, 1, 2
        };

    std::vector<kvs::UInt8> polygonColors =
        {
            200, 200, 255
        };

    const kvs::Xform initializeXform = kvs::Xform(
        kvs::Mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1 ) );

    m_dummy_object = new kvs::PolygonObject();
    m_dummy_object->setName( "Dummy" );
    m_dummy_object->setXform( initializeXform );
    m_dummy_object->setCoords( kvs::ValueArray<kvs::Real32>( polygonCoords ) );
    m_dummy_object->setConnections( kvs::ValueArray<kvs::UInt32>( polygonConnections ) );
    m_dummy_object->setColors( kvs::ValueArray<kvs::UInt8>( polygonColors ) );
    m_dummy_object->setPolygonType( kvs::PolygonObject::Triangle );
    m_dummy_object->setColorType( kvs::PolygonObject::PolygonColor );
    m_dummy_object->setNormalType( kvs::PolygonObject::VertexNormal );
    m_dummy_object->setOpacity( 128 );
    m_screen->registerObject( m_dummy_object, new kvs::StochasticPolygonRenderer() );
}

MainWindow::~MainWindow()
{
    if ( m_compositor && m_compositor->eventTimer() )
    {
        m_compositor->eventTimer()->stop();
        delete m_compositor->eventTimer();
        m_compositor->setEventTimer( nullptr );
    }

    m_web_sockets->binary()->close();
    m_web_sockets->text()->close();
    delete m_viz_mode;
    delete m_web_sockets;
    delete m_time_step_label;
    delete m_fps_label;
    delete m_orientation_axis;
    delete m_color_map_bar;
    delete m_compositor;
    delete m_screen;
    delete ui;
}

void MainWindow::onUpdateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords )
{
    if( m_dummy_object )
    {
        m_dummy_object->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
        m_dummy_object->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );
    }
}

void MainWindow::onUpdateStatusBarMessage( const QString& message )
{
    ui->statusBar->showMessage( message, 5000 );
}

void MainWindow::onUpdateServerState( bool serverState )
{
    if( m_play_back_control_tool_bar ) { m_play_back_control_tool_bar->reset(); }
    if( m_time_step_control_tool_bar ) { m_time_step_control_tool_bar->reset(); }
    if( m_glyph_editor )               { m_glyph_editor->reset(); }
    if( m_object_editor )              { m_object_editor->reset(); }
    if( m_plot_over_line_editor )
    {
        m_plot_over_line_editor_action->setEnabled( serverState );
        m_plot_over_line_editor->reset();
    }
    if( m_plot_over_time_editor )
    {
        m_plot_over_time_editor_action->setEnabled( serverState );
        m_plot_over_time_editor->reset();
    }
    if ( m_ensemble_transfer_function_editor )
    {
        m_ensemble_transfer_function_editor_action->setEnabled( serverState );
        m_ensemble_transfer_function_editor->reset();
    }
    if ( m_transfer_function_editor )
    {
        m_transfer_function_editor_action->setEnabled( serverState );
        m_transfer_function_editor->reset();
    }

    if( serverState ) { return; }

    // NOTE:以下切断時のみ
    if( m_glyph_editor )
    {
        m_glyph_editor_action->setEnabled( false );
        m_glyph_editor->close();
    }
    if( m_plot_over_line_editor )    { m_plot_over_line_editor->close(); }
    if( m_plot_over_time_editor )    { m_plot_over_time_editor->close(); }
    if( m_ensemble_transfer_function_editor ) { m_ensemble_transfer_function_editor->close(); }
    if( m_transfer_function_editor ) { m_transfer_function_editor->close(); }
}

void MainWindow::onUpdateNumberOfVector( const int numberOfVector )
{
    if( m_glyph_editor )
    {
        // NOTE:GlyphEditorは成分数3以上でないといけない
        if( numberOfVector >= 3 )
        {
            m_glyph_editor->onUpdateNumberOfVector( numberOfVector );
            m_glyph_editor_action->setEnabled( true );
        }
        else
        {
            m_glyph_editor_action->setEnabled( false );
        }
    }

    if( m_plot_over_line_editor )
    {
        m_plot_over_line_editor->onUpdateNumberOfVector( numberOfVector );
    }

    if( m_plot_over_time_editor )
    {
        m_plot_over_time_editor->onUpdateNumberOfVector( numberOfVector );
    }
}

void MainWindow::onUpdateIsEnsemble( bool isEnsemble )
{
    if( m_ensemble_transfer_function_editor_action )
    {
        m_ensemble_transfer_function_editor_action->setEnabled( isEnsemble );
    }
    if( m_transfer_function_editor_action )
    {
        m_transfer_function_editor_action->setEnabled( !isEnsemble );
    }
    if( m_color_map_bar_selector_tool_bar )
    {
        m_color_map_bar_selector_tool_bar->setMode(
            isEnsemble
                ? ColorMapSelectorToolBar::Mode::EnsembleTransferFunction
                : ColorMapSelectorToolBar::Mode::TransferFunction );
    }
    if( isEnsemble )
    {
        if( m_ensemble_transfer_function_editor )
        {
            m_ensemble_transfer_function_editor->emitLegendTransferFunctionUpdate();
        }
    }
    else
    {
        if( m_transfer_function_editor )
        {
            m_transfer_function_editor->emitLegendTransferFunctionUpdate();
        }
    }
}

void MainWindow::initializeToolBar()
{
    if( m_color_map_bar_selector_tool_bar )
    {
        m_color_map_bar_selector_tool_bar->setColorMapBar( m_color_map_bar );
    }

    if( m_play_back_control_tool_bar && m_time_step_control_tool_bar )
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
    }

    if( m_time_step_control_tool_bar )
    {
        connect( m_time_step_control_tool_bar, &TimeStepControlToolBar::requestDataAt       , m_object_editor             , &ObjectEditor::onRequestDataAt );
        connect( m_time_step_control_tool_bar, &TimeStepControlToolBar::dataRequestCompleted, m_play_back_control_tool_bar, &PlayBackControlToolBar::onDataRequestCompleted );
        connect( m_time_step_control_tool_bar, &TimeStepControlToolBar::dataRequestCompleted, m_total_particles_tool_bar  , &TotalParticlesToolBar::onUpdateTotalParticles );
        connect( m_time_step_control_tool_bar, &TimeStepControlToolBar::dataRequestCompleted, m_preference                , &Preference::onDataRequestCompleted );
    }

    if( m_play_back_control_tool_bar )
    {
        this->addToolBar( Qt::TopToolBarArea, m_play_back_control_tool_bar );
    }
    if( m_time_step_control_tool_bar )
    {
        this->addToolBarBreak( Qt::TopToolBarArea );
        this->addToolBar( Qt::TopToolBarArea, m_time_step_control_tool_bar );
    }
    if( m_total_particles_tool_bar )
    {
        this->addToolBarBreak( Qt::TopToolBarArea );
        this->addToolBar( Qt::TopToolBarArea, m_total_particles_tool_bar );
    }
    if( m_color_map_bar_selector_tool_bar )
    {
        this->addToolBar( Qt::TopToolBarArea, m_color_map_bar_selector_tool_bar );
    }
}

void MainWindow::initializeMenuBar()
{
    if( m_animation_control )
    {
        m_animation_control_action = new QAction( tr( "Animation Control" ), this );
        connect( m_animation_control_action, &QAction::triggered, this, &MainWindow::onAnimationControl );
    }
    if( m_communication )
    {
        m_communication_action = new QAction( tr( "Communication" ), this );
        connect( m_communication_action, &QAction::triggered, this, &MainWindow::onCommunication );
    }
    if( m_glyph_editor )
    {
        m_glyph_editor_action = new QAction( tr( "Glyph Editor" ), this );
        m_glyph_editor_action->setEnabled( false ); // NOTE:サーバ接続前は無効
        connect( m_glyph_editor_action, &QAction::triggered, this, &MainWindow::onGlyphEditor );
    }
    if( m_object_editor )
    {
        m_object_editor_action = new QAction( tr( "Object Editor" ), this );
        connect( m_object_editor_action, &QAction::triggered, this, &MainWindow::onObjectEditor );
    }
    if( m_plot_over_line_editor )
    {
        m_plot_over_line_editor_action = new QAction( tr( "Plot Over Line Editor" ), this );
        m_plot_over_line_editor_action->setEnabled( false ); // NOTE:サーバ接続前は無効
        connect( m_plot_over_line_editor_action, &QAction::triggered, this, &MainWindow::onPlotOverLineEditor );
    }
    if( m_plot_over_time_editor )
    {
        m_plot_over_time_editor_action = new QAction( tr( "Plot Over Time Editor" ), this );
        m_plot_over_time_editor_action->setEnabled( false ); // NOTE:サーバ接続前は無効
        connect( m_plot_over_time_editor_action, &QAction::triggered, this, &MainWindow::onPlotOverTimeEditor );
    }
    if( m_point_size_control )
    {
        m_point_size_control_action = new QAction( tr( "Point Size Control" ), this );
        connect( m_point_size_control_action, &QAction::triggered, this, &MainWindow::onPointSizeControl );
    }
    if( m_preference )
    {
        m_preference_action = new QAction( tr( "Preference" ), this );
        connect( m_preference_action, &QAction::triggered, this, &MainWindow::onPreference );
        ui->menuPBVRClient->addAction( m_preference_action );
    }
    if( m_repetition_level_control )
    {
        m_repetition_level_control_action = new QAction( tr( "Repetition Level Control" ), this );
        connect( m_repetition_level_control_action, &QAction::triggered, this, &MainWindow::onRepetitionLevelControl );
    }
    if( m_shading_control )
    {
        m_shading_control_action = new QAction( tr( "Shading Control" ), this );
        connect( m_shading_control_action, &QAction::triggered, this, &MainWindow::onShadingControl );
    }
    if( m_ensemble_transfer_function_editor )
    {
        m_ensemble_transfer_function_editor_action = new QAction( tr( "Ensemble Transfer Function Editor" ), this );
        m_ensemble_transfer_function_editor_action->setEnabled( false );
        connect( m_ensemble_transfer_function_editor_action, &QAction::triggered, this, &MainWindow::onEnsembleTransferFunctionEditor );
    }
    if( m_transfer_function_editor )
    {
        m_transfer_function_editor_action = new QAction( tr( "Transfer Function Editor" ), this );
        m_transfer_function_editor_action->setEnabled( false ); // NOTE:サーバ接続前は無効
        connect( m_transfer_function_editor_action, &QAction::triggered, this, &MainWindow::onTransferFunctionEditor );
    }
    if( m_volume_transform )
    {
        m_volume_transform_action = new QAction( tr( "Volume Transform" ), this );
        connect( m_volume_transform_action, &QAction::triggered, this, &MainWindow::onVolumeTransform );
    }

    auto add = [&]( QMenu* menu, QAction* action )
    {
        if( action ) { menu->addAction( action ); }
    };

    add( ui->menuTools, m_communication_action );
    if( m_communication_action && (
            m_animation_control_action        ||
            m_glyph_editor_action             ||
            m_object_editor_action            ||
            m_plot_over_line_editor_action    ||
            m_plot_over_time_editor_action    ||
            m_point_size_control_action       ||
            m_repetition_level_control_action ||
            m_shading_control_action          ||
            m_ensemble_transfer_function_editor_action ||
            m_transfer_function_editor_action ||
            m_volume_transform_action ) )
    {
        ui->menuTools->addSeparator();
    }
    add( ui->menuTools, m_animation_control_action );
    add( ui->menuTools, m_glyph_editor_action );
    add( ui->menuTools, m_object_editor_action );
    add( ui->menuTools, m_plot_over_line_editor_action );
    add( ui->menuTools, m_plot_over_time_editor_action );
    add( ui->menuTools, m_point_size_control_action );
    add( ui->menuTools, m_repetition_level_control_action );
    add( ui->menuTools, m_shading_control_action );
    add( ui->menuTools, m_ensemble_transfer_function_editor_action );
    add( ui->menuTools, m_transfer_function_editor_action );
    add( ui->menuTools, m_volume_transform_action );
}


void MainWindow::initializeAnimationControl()
{
    if( m_animation_control )
    {
        m_animation_control->adjustSize();
        m_animation_control->close();
        addDockWidget( Qt::RightDockWidgetArea, m_animation_control );
    }
}

void MainWindow::initializeCommunication()
{
    if( m_communication )
    {
        m_communication->adjustSize();
        addDockWidget( Qt::RightDockWidgetArea, m_communication );

        connect( m_communication, &Communication::updateStatusBarMessage, this, &MainWindow::onUpdateStatusBarMessage );

        connect( m_communication, &Communication::updateServerState, this, &MainWindow::onUpdateServerState );

        connect( m_communication, &Communication::updateOperatorState, m_play_back_control_tool_bar, &PlayBackControlToolBar::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState, m_time_step_control_tool_bar, &TimeStepControlToolBar::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState, m_glyph_editor              , &GlyphEditor           ::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState, m_object_editor             , &ObjectEditor          ::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState, m_plot_over_line_editor     , &PlotOverLineEditor    ::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState, m_plot_over_time_editor     , &PlotOverTimeEditor    ::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState, m_transfer_function_editor  , &TransferFunctionEditor::onOperatorStateUpdate );
        connect( m_communication, &Communication::updateOperatorState, m_repetition_level_control  , &RepetitionLevelControl::onOperatorStateUpdate );

        // NOTE:バイナリソケット用
        connect( m_communication, &Communication::unpack, m_object_editor, &ObjectEditor::onUnpack );

        // NOTE:テキストソケット用        
        connect( m_communication, &Communication::receiveTimeStepControlParameter , m_time_step_control_tool_bar, &TimeStepControlToolBar::onReceiveTimeStepControlParameter );
        connect( m_communication, &Communication::receiveGlyphParameter           , m_glyph_editor              , &GlyphEditor           ::onReceiveGlyphParameter );
        connect( m_communication, &Communication::receiveObjectInfoParameter      , m_object_editor             , &ObjectEditor          ::onReceiveObjectInfoParameter );
        connect( m_communication, &Communication::receivePlotOverLineParameter    , m_plot_over_line_editor     , &PlotOverLineEditor    ::onReceivePlotOverLineParameter );
        connect( m_communication, &Communication::receivePlotOverTimeParameter    , m_plot_over_time_editor     , &PlotOverTimeEditor    ::onReceivePlotOverTimeParameter );
        connect( m_communication, &Communication::receiveTransferFunctionParameter, m_transfer_function_editor  , &TransferFunctionEditor::onReceiveTransferFunctionParameter );
        connect( m_communication, &Communication::receiveEnsembleStatisticsParameter, m_ensemble_transfer_function_editor, &EnsembleTransferFunctionEditor::onReceiveEnsembleStatisticsParameter );
        connect( m_communication, &Communication::receiveRepetitionLevelParameter , m_repetition_level_control  , &RepetitionLevelControl::onReceiveRepetitionLevelParameter );

        connect( m_communication, &Communication::receiveRequestDataAtTransferFunctionParameter, m_transfer_function_editor, &TransferFunctionEditor::onReceiveRequestDataAtTransferFunctionParameter );
        connect( m_communication, &Communication::receiveRequestDataAtEnsembleStatisticsParameter, m_ensemble_transfer_function_editor, &EnsembleTransferFunctionEditor::onReceiveRequestDataAtEnsembleStatisticsParameter );

        connect( m_communication, &Communication::receiveSelectedFile, m_object_editor, &ObjectEditor::onReceiveSelectedFile );
        connect( m_communication, &Communication::receiveObjectDelete, m_object_editor, &ObjectEditor::onReceiveObjectDelete );

        connect( m_communication, &Communication::updateObjectLatestTimeStep, m_object_editor, &ObjectEditor::onUpdateObjectLatestTimeStep );
    }
}

void MainWindow::initializeGlyphEditor()
{
    if( m_glyph_editor )
    {
        connect( m_glyph_editor, &GlyphEditor::glyphParameterUpdate, m_object_editor, &ObjectEditor::onGlyphParameterUpdate );
    }
}

void MainWindow::initializeObjectEditor()
{
    if( m_object_editor )
    {
        m_object_editor->adjustSize();
        addDockWidget( Qt::LeftDockWidgetArea, m_object_editor );

        connect( m_object_editor, &ObjectEditor::updateStatusBarMessage, this, &MainWindow::onUpdateStatusBarMessage );

        connect( m_object_editor, &ObjectEditor::updateNumberOfVector, this, &MainWindow::onUpdateNumberOfVector );
        connect( m_object_editor, &ObjectEditor::updateIsEnsemble, this, &MainWindow::onUpdateIsEnsemble );

        connect( m_object_editor, &ObjectEditor::updateTotalTimeStepRange, m_time_step_control_tool_bar, &TimeStepControlToolBar::onUpdateTotalTimeStepRange );

        connect( m_object_editor, &ObjectEditor::updateFocus             , this                        , &MainWindow::onUpdateFocus );
        connect( m_object_editor, &ObjectEditor::updateFocus             , m_communication             , &Communication::onUpdateFocus );
        connect( m_object_editor, &ObjectEditor::updateFocus             , m_plot_over_line_editor     , &PlotOverLineEditor::onUpdateFocus );
        connect( m_object_editor, &ObjectEditor::updateFocus             , m_plot_over_time_editor     , &PlotOverTimeEditor::onUpdateFocus );

        connect( m_object_editor, &ObjectEditor::updateTranslation       , m_plot_over_line_editor     , &PlotOverLineEditor::onUpdateTranslation );
        connect( m_object_editor, &ObjectEditor::updateTranslation       , m_plot_over_time_editor     , &PlotOverTimeEditor::onUpdateTranslation );

        connect( m_object_editor, &ObjectEditor::shading, m_shading_control, &ShadingControl::onShading );

        connect( m_object_editor, &ObjectEditor::dataRequestCompleted    , m_time_step_control_tool_bar, &TimeStepControlToolBar::onDataRequestCompleted );
    }
}

void MainWindow::initializePlotOverLineEditor()
{
    if( m_plot_over_line_editor )
    {
        m_plot_over_line_editor->adjustSize();
        m_plot_over_line_editor->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_plot_over_line_editor );
    }
}

void MainWindow::initializePlotOverTimeEditor()
{
    if( m_plot_over_time_editor )
    {
        m_plot_over_time_editor->adjustSize();
        m_plot_over_time_editor->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_plot_over_time_editor );
    }
}

void MainWindow::initializePointSizeControl()
{
    if( m_point_size_control )
    {
        m_point_size_control->adjustSize();
        m_point_size_control->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_point_size_control );
    }
}

void MainWindow::initializePreference()
{
    if( m_preference )
    {
        m_preference->adjustSize();
        m_preference->setScreen( m_screen );
        m_preference->setCompositor( m_compositor );
        m_preference->setColorMapBar( m_color_map_bar );
        m_preference->setOrientationAxis( m_orientation_axis );
        m_preference->setFpsLabel( m_fps_label );
        m_preference->setTimeStepLabel( m_time_step_label );
    }
}

void MainWindow::initializeRepetitionLevelControl()
{
    if( m_repetition_level_control )
    {
        m_repetition_level_control->adjustSize();
        m_repetition_level_control->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_repetition_level_control );

        connect( m_repetition_level_control, &RepetitionLevelControl::shading, m_shading_control, &ShadingControl::onShading );

        emit updateInitialRepetitionLevel();
    }
}

void MainWindow::initializeShadingControl()
{
    if( m_shading_control )
    {
        m_shading_control->adjustSize();
        m_shading_control->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_shading_control );
    }
}

void MainWindow::initializeEnsembleTransferFunctionEditor()
{
    if( m_ensemble_transfer_function_editor )
    {
        m_ensemble_transfer_function_editor->adjustSize();
        m_ensemble_transfer_function_editor->close();
        if( m_object_editor )
        {
            m_object_editor->setEnsembleTransferFunctionEditor( m_ensemble_transfer_function_editor );
        }
        connect(
            m_ensemble_transfer_function_editor,
            &EnsembleTransferFunctionEditor::updateLegendTransferFunction,
            m_color_map_bar_selector_tool_bar,
            &ColorMapSelectorToolBar::onEnsembleTransferFunctionUpdate );
    }
}

void MainWindow::initializeTransferFunctionEditor()
{
    if( m_transfer_function_editor )
    {

        m_transfer_function_editor->adjustSize();

        connect( m_transfer_function_editor, &TransferFunctionEditor::transferFunctionUpdate, m_object_editor, &ObjectEditor::onTransferFunctionUpdate );
        connect( m_transfer_function_editor, &TransferFunctionEditor::updateLastSentTransferFunction, m_color_map_bar_selector_tool_bar, &ColorMapSelectorToolBar::onTransferFunctionUpdate );
    }
}

void MainWindow::initializeVolumeTransform()
{
    if( m_volume_transform )
    {
        m_volume_transform->adjustSize();
        m_volume_transform->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_volume_transform );

        connect( m_volume_transform, &VolumeTransform::updateTranslation, m_plot_over_line_editor, &PlotOverLineEditor::onUpdateTranslation );
        connect( m_volume_transform, &VolumeTransform::updateTranslation, m_plot_over_time_editor, &PlotOverTimeEditor::onUpdateTranslation );
    }
}

void MainWindow::initializeShaderAndFonts()
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
