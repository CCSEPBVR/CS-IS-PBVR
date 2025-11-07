#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow( kvs::qt::Application& app, QWidget *parent )
    : QMainWindow(parent)
    , ui( new Ui::MainWindow )    
    , m_screen( new kvs::qt::jaea::Screen( &app ) )
    , m_compositor( new kvs::StochasticRenderingCompositor( m_screen->scene() ) )
    , m_color_map_bar( nullptr )
    , m_orientation_axis( nullptr )
    , m_fps_label( nullptr )
    , m_time_step_label( nullptr )
    , m_web_sockets( new WebSocketPair )
    // ウィジェット群(A~Z)
    // ABCDEFGHIJKLMNOPQRSTUVWXYZ
    , m_color_map_bar_selector_tool_bar( new ColorMapSelectorToolBar( m_screen, this ) )
    , m_play_back_control_tool_bar( new PlayBackControlToolBar( this ) )
    , m_time_step_control_tool_bar( new TimeStepControlToolBar( this ) )
    , m_total_particles_tool_bar( new TotalParticlesToolBar( this ) )
    , m_animation_control( new AnimationControl( m_screen, this ) )
    , m_communication( new Communication( m_screen, m_web_sockets, this ) )
    , m_glyph_editor( new GlyphEditor( m_web_sockets, this ) )
    , m_object_editor( new ObjectEditorWIP( m_web_sockets, m_screen, this ) )
    , m_plot_over_line_editor( new PlotOverLineEditor( m_web_sockets, m_screen, this ) )
    , m_point_size_control( new PointSizeControl( m_screen, this ) )
    , m_preference( new Preference( this ) )
    , m_repetition_level_control( new RepetitionLevelControl( m_screen, m_compositor, this ) )
    , m_shading_control( new ShadingControl( m_screen, this ) )
    , m_transfer_function_editor( new TransferFunctionEditor( m_web_sockets, this ) )
    , m_volume_transform( new VolumeTransform( m_screen, this ) )
    , m_initialize_camera_xform( kvs::Mat4(
          1, 0, 0, 0 ,
          0, 1, 0, 0 ,
          0, 0, 1, 12,
          0, 0, 0, 1
          ) )
{
    initialize();
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

void MainWindow::updateStatusBarMessage( const QString& message )
{
    ui->statusBar->showMessage( message, 5000 ); // 5秒間表示
}

void MainWindow::initialize()
{    
    ui->setupUi( this ); // UIのセットアップ

    setWindowTitle( QString("pbvr_client - %1").arg( GIT_BRANCH_NAME ) );

    m_compositor->setRepetitionLevel( 4 ); // コンポジターのリピートレベルを設定 初期値:4
    m_screen->setEvent( m_compositor );

    m_screen->setFixedSize( 620, 620 );
    ui->screenArea->addWidget( m_screen );

    m_color_map_bar = new kvs::ColorMapBar( m_screen );
    m_orientation_axis = new kvs::OrientationAxis( m_screen, m_screen->scene() );
    m_fps_label = new kvs::Label( m_screen );
    m_time_step_label = new kvs::Label( m_screen );

    m_load_action = new QAction( tr( "Load"), this );
    connect( m_load_action, &QAction::triggered, this, &MainWindow::onLoad );
    ui->menupbvr_client->addAction( m_load_action );

    m_save_action = new QAction( tr( "Save"), this );
    connect( m_save_action, &QAction::triggered, this, &MainWindow::onSave );
    ui->menupbvr_client->addAction( m_save_action );

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

    // デバッグボタン作成
    QPushButton* debugPushButton = new QPushButton( "Debug", this );
    debugPushButton->setGeometry( 650, 50, 120, 40 );
    debugPushButton->show();

    // ボタン押下でサーバにイベント送信
    connect( debugPushButton, &QPushButton::clicked, this, [this]() {
        RemoteFileDialog dlg( m_web_sockets->text(), this );
        if( dlg.exec() == QDialog::Accepted )
        {
            qDebug() << "選択ファイル:" << dlg.selectedFile();
        }
    } );
}

void MainWindow::toolBarInitialize()
{
    // ツールバーの配置
    if( m_time_step_control_tool_bar )
    {
        connect( m_time_step_control_tool_bar, &TimeStepControlToolBar::requestMerge, m_object_editor, &ObjectEditorWIP::showAtTimeStep );
        connect( m_time_step_control_tool_bar, &TimeStepControlToolBar::done, m_play_back_control_tool_bar, &PlayBackControlToolBar::donePlayBackControlToolBar );
        connect( this, &MainWindow::load, m_time_step_control_tool_bar , &TimeStepControlToolBar::loadParameter );
        connect( this, &MainWindow::save, m_time_step_control_tool_bar , &TimeStepControlToolBar::saveParameter );

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

        connect( this, &MainWindow::load, m_color_map_bar_selector_tool_bar , &ColorMapSelectorToolBar::loadParameter );
        connect( this, &MainWindow::save, m_color_map_bar_selector_tool_bar , &ColorMapSelectorToolBar::saveParameter );

        this->addToolBar( Qt::TopToolBarArea, m_color_map_bar_selector_tool_bar );
        this->addToolBarBreak( Qt::TopToolBarArea );
    }

    if( m_play_back_control_tool_bar )
    {
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::first   , m_time_step_control_tool_bar, &TimeStepControlToolBar::first );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::previous, m_time_step_control_tool_bar, &TimeStepControlToolBar::previous );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::reverse , m_time_step_control_tool_bar, &TimeStepControlToolBar::reverse );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::play    , m_time_step_control_tool_bar, &TimeStepControlToolBar::play );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::next    , m_time_step_control_tool_bar, &TimeStepControlToolBar::next );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::last    , m_time_step_control_tool_bar, &TimeStepControlToolBar::last );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::keepLast, m_time_step_control_tool_bar, &TimeStepControlToolBar::keepLast );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::jump    , m_time_step_control_tool_bar, &TimeStepControlToolBar::jump );
        connect( m_play_back_control_tool_bar, &PlayBackControlToolBar::loop    , m_time_step_control_tool_bar, &TimeStepControlToolBar::loop );
        // connect( this, &MainWindow::load, m_play_back_control_tool_bar , &PlayBackControlToolBar::loadParameter );
        // connect( this, &MainWindow::save, m_play_back_control_tool_bar , &PlayBackControlToolBar::saveParameter );

        this->addToolBar( Qt::TopToolBarArea, m_play_back_control_tool_bar );
    }
}

void MainWindow::animationControlInitialize()
{
    if( m_animation_control )
    {
        m_animation_control_action = new QAction( tr( "Animation Control" ), this );

        connect( m_screen , &kvs::qt::jaea::Screen::addKeyFrameAdd     , m_animation_control , &AnimationControl::addKeyFrameAdd );
        connect( m_screen , &kvs::qt::jaea::Screen::removeLastKeyFrame , m_animation_control , &AnimationControl::removeLastKeyFrame );
        connect( m_screen , &kvs::qt::jaea::Screen::clearKeyFrame      , m_animation_control , &AnimationControl::clearKeyFrame );
        connect( m_screen , &kvs::qt::jaea::Screen::playKeyFrame       , m_animation_control , &AnimationControl::playKeyFrame );
        connect( m_screen , &kvs::qt::jaea::Screen::loadKeyFrameFile   , m_animation_control , &AnimationControl::loadKeyFrameFile );
        connect( m_screen , &kvs::qt::jaea::Screen::saveKeyFrameFile   , m_animation_control , &AnimationControl::saveKeyFrameFile );

        connect( this, &MainWindow::load, m_animation_control , &AnimationControl::loadParameter );
        connect( this, &MainWindow::save, m_animation_control , &AnimationControl::saveParameter );

        connect( m_animation_control_action, &QAction::triggered, this, &MainWindow::onAnimationControl );
        ui->menuTools->addAction( m_animation_control_action );

        m_animation_control->adjustSize();        
        m_animation_control->close();
        addDockWidget( Qt::RightDockWidgetArea, m_animation_control );
    }
}

void MainWindow::communicationInitialize()
{
    if( m_communication )
    {
        m_communication_action = new QAction( tr( "Communication" ), this );

        connect( this, &MainWindow::load, m_communication , &Communication::loadParameter );
        connect( this, &MainWindow::save, m_communication , &Communication::saveParameter );

        connect( m_communication, &Communication::updateStatusBarMessage, this, &MainWindow::updateStatusBarMessage );
        connect( m_communication, &Communication::updateServerState, this, &MainWindow::onUpdateServerState );
        connect( m_communication, &Communication::updateOperatorState, m_glyph_editor, &GlyphEditor::updateOperatorState );
        connect( m_communication, &Communication::updateOperatorState, m_object_editor, &ObjectEditorWIP::updateOperatorState );
        connect( m_communication, &Communication::updateOperatorState, m_plot_over_line_editor, &PlotOverLineEditor::updateOperatorState );
        connect( m_communication, &Communication::updateOperatorState, m_transfer_function_editor, &TransferFunctionEditor::updateOperatorState );

        connect( m_communication_action, &QAction::triggered, this, &MainWindow::onCommunication );

        // メニューの先頭に挿入
        if( !ui->menuTools->actions().isEmpty() )
        {
            ui->menuTools->insertAction( ui->menuTools->actions().first(), m_communication_action );
        }
        else
        {
            ui->menuTools->addAction( m_communication_action );
        }
        ui->menuTools->addSeparator();

        m_communication->adjustSize();
        addDockWidget( Qt::RightDockWidgetArea, m_communication );
    }
}

void MainWindow::glyphEditorInitialize()
{
    if( m_glyph_editor )
    {
        m_glyph_editor_action = new QAction( tr( "Glyph Editor"), this );

        connect( this, &MainWindow::load, m_glyph_editor, &GlyphEditor::loadParameter );
        connect( this, &MainWindow::save, m_glyph_editor, &GlyphEditor::saveParameter );

        connect( m_glyph_editor_action, &QAction::triggered, this, &MainWindow::onGlyphEditor );

        m_glyph_editor_action->setEnabled( false ); // サーバ接続前は無効

        ui->menuTools->addAction( m_glyph_editor_action );
        // m_glyph_editor->updateNumberOfVector( 3 ); // DEBUG:成分数に応じてUIが変化するか確認
    }
}

void MainWindow::objectEditorInitialize()
{
    if( m_object_editor )
    {
        m_object_editor_action = new QAction( tr( "Object Editor"), this );

        connect( m_object_editor, &ObjectEditorWIP::updateTotalTimeStepRange, m_time_step_control_tool_bar, &TimeStepControlToolBar::updateTotalTimeStepRange );
        connect( m_object_editor, &ObjectEditorWIP::updateFocus             , m_plot_over_line_editor, &PlotOverLineEditor::updateFocus );
        connect( m_object_editor, &ObjectEditorWIP::updateTranslation       , m_plot_over_line_editor, &PlotOverLineEditor::updateTranslation );
        connect( m_object_editor, &ObjectEditorWIP::shading                 , m_shading_control, &ShadingControl::shading );
        connect( m_object_editor, &ObjectEditorWIP::done                    , m_time_step_control_tool_bar, &TimeStepControlToolBar::doneTimeControlToolBar );
        // connect( m_object_editor, &ObjectEditorWIP::noItems                         , m_time_step_control_tool_bar, &TimeStepControlToolBar::noItems );
        // connect( m_object_editor, &ObjectEditorWIP::updateInSituObjectMinMaxTimeStep, m_time_step_control_tool_bar, &TimeStepControlToolBar::updateInSituObjectMinMaxTimeStep );
        connect( this, &MainWindow::load, m_object_editor, &ObjectEditorWIP::loadParameter );
        connect( this, &MainWindow::save, m_object_editor, &ObjectEditorWIP::saveParameter );

        connect( m_object_editor_action, &QAction::triggered, this, &MainWindow::onObjectEditor );

        // m_object_editor_action->setEnabled( false ); // サーバ接続前は無効

        ui->menuTools->addAction( m_object_editor_action );

        m_object_editor->adjustSize();
        addDockWidget( Qt::LeftDockWidgetArea, m_object_editor );
    }
}

void MainWindow::plotOverLineEditorInitialize()
{
    if( m_plot_over_line_editor )
    {
        m_plot_over_line_editor_action = new QAction( tr( "Plot Over Line Editor"), this );

        connect( m_screen , &kvs::qt::jaea::Screen::updateTranslation   , m_plot_over_line_editor , &PlotOverLineEditor::updateTranslation );

        connect( this, &MainWindow::load, m_plot_over_line_editor, &PlotOverLineEditor::loadParameter );
        connect( this, &MainWindow::save, m_plot_over_line_editor, &PlotOverLineEditor::saveParameter );

        connect( m_plot_over_line_editor_action, &QAction::triggered, this, &MainWindow::onPlotOverLineEditor );

        m_plot_over_line_editor_action->setEnabled( false ); // サーバ接続前は無効

        ui->menuTools->addAction( m_plot_over_line_editor_action );

        m_plot_over_line_editor->adjustSize();
        m_plot_over_line_editor->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_plot_over_line_editor );
        // m_plot_over_line_editor->updateNumberOfVector( 3 ); // DEBUG:成分数に応じてUIが変化するか確認
    }
}

void MainWindow::pointSizeControlInitialize()
{
    if( m_point_size_control )
    {
        m_point_size_control_action = new QAction( tr( "Point Size Control"), this );

        connect( this, &MainWindow::load, m_point_size_control, &PointSizeControl::loadParameter );
        connect( this, &MainWindow::save, m_point_size_control, &PointSizeControl::saveParameter );

        connect( m_point_size_control_action, &QAction::triggered, this, &MainWindow::onPointSizeControl );

        ui->menuTools->addAction( m_point_size_control_action );

        m_point_size_control->adjustSize();        
        m_point_size_control->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_point_size_control );
    }
}

void MainWindow::preferenceInitialize()
{
    if( m_preference )
    {
        m_preference_action = new QAction( tr( "Preference"), this );

        m_preference->setScreen( m_screen );
        m_preference->setCompositor( m_compositor );
        m_preference->setColorMapBar( m_color_map_bar );
        m_preference->setOrientationAxis( m_orientation_axis );
        m_preference->setFpsLabel( m_fps_label );
        m_preference->setTimeStepLabel( m_time_step_label );
        connect( this, &MainWindow::readyScreen, m_preference, &Preference::readyScreen );

        connect( m_preference_action, &QAction::triggered, this, &MainWindow::onPreference );

        ui->menupbvr_client->addAction( m_preference_action );
        m_preference->adjustSize();
    }
}

void MainWindow::repetitionLevelControlInitialize()
{
    if( m_repetition_level_control )
    {
        m_repetition_level_control_action = new QAction( tr( "Repetition Level Control"), this );

        connect( this, &MainWindow::updateCurrentRepetitionLevel, m_repetition_level_control, &RepetitionLevelControl::updateCurrentRepetitionLevel );
        connect( this, &MainWindow::load, m_repetition_level_control, &RepetitionLevelControl::loadParameter );
        connect( this, &MainWindow::save, m_repetition_level_control, &RepetitionLevelControl::saveParameter );

        connect( m_repetition_level_control , &RepetitionLevelControl::shading , m_shading_control , &ShadingControl::shading );

        connect( m_repetition_level_control_action, &QAction::triggered, this, &MainWindow::onRepetitionLevelControl );
        emit updateCurrentRepetitionLevel();

        ui->menuTools->addAction( m_repetition_level_control_action );

        m_repetition_level_control->adjustSize();        
        m_repetition_level_control->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_repetition_level_control );
    }
}

void MainWindow::shadingControlInitialize()
{
    if( m_shading_control )
    {
        m_shading_control_action = new QAction( tr( "Shading Control"), this );

        connect( this, &MainWindow::load, m_shading_control, &ShadingControl::loadParameter );
        connect( this, &MainWindow::save, m_shading_control, &ShadingControl::saveParameter );

        connect( m_shading_control_action, &QAction::triggered, this, &MainWindow::onShadingControl );

        ui->menuTools->addAction( m_shading_control_action );
        m_shading_control->adjustSize();        
        m_shading_control->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_shading_control );
    }
}

void MainWindow::transferFunctionEditorInitialize()
{
    if( m_transfer_function_editor )
    {
        m_transfer_function_editor_action = new QAction( tr( "Transfer Function Editor"), this );

        connect( this, &MainWindow::load, m_transfer_function_editor, &TransferFunctionEditor::loadParameter );
        connect( this, &MainWindow::save, m_transfer_function_editor, &TransferFunctionEditor::saveParameter );

        connect( m_transfer_function_editor_action, &QAction::triggered, this, &MainWindow::onTransferFunctionEditor );

        m_transfer_function_editor_action->setEnabled( false ); // サーバ接続前は無効

        ui->menuTools->addAction( m_transfer_function_editor_action );
    }
    // connect( m_transfer_function_editor         , &TransferFunctionEditor::updateColorMapBar                    , m_color_map_bar_selector_tool_bar , &ColorMapSelectorToolBar::updateColorMapBar );
    // connect( m_transfer_function_editor         , &TransferFunctionEditor::failedTransferFunctionImport         , m_connect                         , &Connect::failedTransferFunctionImport );
    // connect( m_transfer_function_editor         , &TransferFunctionEditor::successTransferFunctionImport        , m_connect                         , &Connect::successTransferFunctionImport );
    // connect( m_transfer_function_editor         , &TransferFunctionEditor::updateTransferFunctionClientMessage  , m_connect                         , &Connect::updateTransferFunctionClientMessage );
    // connect( m_transfer_function_editor         , &TransferFunctionEditor::requestReplaceServerPointObject      , m_object_editor                   , &ObjectEditor::requestReplaceServerPointObject );
}

void MainWindow::volumeTransformInitialize()
{
    if( m_volume_transform )
    {
        m_volume_transform_action = new QAction( tr( "Volume Transform"), this );

        connect( this, &MainWindow::load, m_volume_transform, &VolumeTransform::loadParameter );
        connect( this, &MainWindow::save, m_volume_transform, &VolumeTransform::saveParameter );

        connect( m_volume_transform_action, &QAction::triggered, this, &MainWindow::onVolumeTransform );

        ui->menuTools->addAction( m_volume_transform_action );

        m_volume_transform->adjustSize();        
        m_volume_transform->close();
        addDockWidget( Qt::LeftDockWidgetArea, m_volume_transform );
    }
}

void MainWindow::initializeAfterShow()
{
    QString shaderDIR = QDir( QCoreApplication::applicationDirPath() ).filePath( "Shader" );
    kvs::ShaderSource::AddSearchPath( shaderDIR.toUtf8().constData() );

    QString fontDIR = QDir(QCoreApplication::applicationDirPath()).filePath("Font/");

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
    // ウィジェット群(A~Z)
    // ABCDEFGHIJKLMNOPQRSTUVWXYZ

    if( m_glyph_editor && m_glyph_editor_action )
    {
        // TODO:サーバーと導通時にサーバから成分数を送ってもらう必要がある。 成分数が3未満の場合、GlyphEditorは開けなくする必要がある。
        m_glyph_editor_action->setEnabled( serverState );
        if( !serverState )
        {
            m_glyph_editor->close();
            m_glyph_editor->reset();
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
            m_plot_over_line_editor->reset();
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
    QString fileName = QFileDialog::getOpenFileName( this, tr("Load File"), QString(), tr("JSON Files (*.json);;All Files (*)") );
    emit load( fileName );
}

void MainWindow::onSave()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr("Load File"), QString(), tr("JSON Files (*.json);;All Files (*)") );
    emit save( fileName );
}
