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
    , m_web_binary_socket( new QWebSocket() )
    , m_web_text_socket( new QWebSocket() )
    // ウィジェット群(A~Z)
    // ABCDEFGHIJKLMNOPQRSTUVWXYZ
    , m_animation_control( new AnimationControl( m_screen, this ) )
    , m_communication( new Communication( m_screen, m_web_binary_socket, m_web_text_socket, this ) )
    , m_glyph_editor( new GlyphEditor( m_web_text_socket, this ) )
    , m_plot_over_line_editor( new PlotOverLineEditor( m_web_text_socket, m_screen, this ) )
    , m_point_size_control( new PointSizeControl( m_screen, this ) )
    , m_preference( new Preference( this ) )
    , m_repetition_level_control( new RepetitionLevelControl( m_screen, m_compositor, this ) )
    , m_shading_control( new ShadingControl( m_screen, this ) )
    , m_transfer_function_editor( new TransferFunctionEditor( m_web_text_socket, this ) )
    , m_volume_transform( new VolumeTransform( m_screen, this ) )
{
    initialize();
}

MainWindow::~MainWindow()
{
    delete m_web_text_socket;
    delete m_web_binary_socket;
    delete m_time_step_label;
    delete m_fps_label;
    delete m_orientation_axis;
    delete m_color_map_bar;
    delete m_compositor;
    delete m_screen;
    delete ui;
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

    m_web_binary_socket->setParent( this );
    m_web_text_socket->setParent( this );

    animationControlInitialize();
    communicationInitialize();
    glyphEditorInitialize();
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

void MainWindow::animationControlInitialize()
{
    if( m_animation_control )
    {
        m_animation_control_action = new QAction( tr( "Animation Control" ), this );
        connect( m_animation_control_action, &QAction::triggered, this, &MainWindow::onAnimationControl );
        connect( m_screen , &kvs::qt::jaea::Screen::addKeyFrameAdd     , m_animation_control , &AnimationControl::addKeyFrameAdd );
        connect( m_screen , &kvs::qt::jaea::Screen::removeLastKeyFrame , m_animation_control , &AnimationControl::removeLastKeyFrame );
        connect( m_screen , &kvs::qt::jaea::Screen::clearKeyFrame      , m_animation_control , &AnimationControl::clearKeyFrame );
        connect( m_screen , &kvs::qt::jaea::Screen::playKeyFrame       , m_animation_control , &AnimationControl::playKeyFrame );
        connect( m_screen , &kvs::qt::jaea::Screen::loadKeyFrameFile   , m_animation_control , &AnimationControl::loadKeyFrameFile );
        connect( m_screen , &kvs::qt::jaea::Screen::saveKeyFrameFile   , m_animation_control , &AnimationControl::saveKeyFrameFile );

        ui->menuTools->addAction( m_animation_control_action );
        m_animation_control->adjustSize();
        addDockWidget( Qt::RightDockWidgetArea, m_animation_control );
        m_animation_control->close();
    }
}

void MainWindow::communicationInitialize()
{
    if( m_communication )
    {
        m_communication_action = new QAction( tr( "Communication" ), this );
        connect( m_communication_action, &QAction::triggered, this, &MainWindow::onCommunication );
        connect( m_communication, &Communication::updateServerState, this, &MainWindow::onUpdateServerState );
        connect( m_communication, &Communication::updateOperatorState, m_glyph_editor, &GlyphEditor::updateOperatorState );
        connect( m_communication, &Communication::updateOperatorState, m_plot_over_line_editor, &PlotOverLineEditor::updateOperatorState );
        connect( m_communication, &Communication::updateOperatorState, m_transfer_function_editor, &TransferFunctionEditor::updateOperatorState );

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
        connect( m_glyph_editor_action, &QAction::triggered, this, &MainWindow::onGlyphEditor );

        m_glyph_editor_action->setEnabled( false ); // サーバ接続前は無効

        ui->menuTools->addAction( m_glyph_editor_action );
        // m_glyph_editor->updateNumberOfVector( 3 ); // DEBUG:成分数に応じてUIが変化するか確認
    }
}

void MainWindow::plotOverLineEditorInitialize()
{
    if( m_plot_over_line_editor )
    {
        m_plot_over_line_editor_action = new QAction( tr( "Plot Over Line Editor"), this );
        connect( m_plot_over_line_editor_action, &QAction::triggered, this, &MainWindow::onPlotOverLineEditor );

        m_plot_over_line_editor_action->setEnabled( false ); // サーバ接続前は無効

        ui->menuTools->addAction( m_plot_over_line_editor_action );
        // m_plot_over_line_editor->updateNumberOfVector( 3 ); // DEBUG:成分数に応じてUIが変化するか確認

        m_plot_over_line_editor->adjustSize();
        addDockWidget( Qt::LeftDockWidgetArea, m_plot_over_line_editor );
        m_plot_over_line_editor->close();
    }
}

void MainWindow::pointSizeControlInitialize()
{
    if( m_point_size_control )
    {
        m_point_size_control_action = new QAction( tr( "Point Size Control"), this );
        connect( m_point_size_control_action, &QAction::triggered, this, &MainWindow::onPointSizeControl );

        ui->menuTools->addAction( m_point_size_control_action );

        m_point_size_control->adjustSize();
        addDockWidget( Qt::LeftDockWidgetArea, m_point_size_control );
        m_point_size_control->close();
    }
}

void MainWindow::preferenceInitialize()
{
    if( m_preference )
    {
        m_preference->setScreen( m_screen );
        m_preference->setCompositor( m_compositor );
        m_preference->setColorMapBar( m_color_map_bar );
        m_preference->setOrientationAxis( m_orientation_axis );
        m_preference->setFpsLabel( m_fps_label );
        m_preference->setTimeStepLabel( m_time_step_label );
        connect( this, &MainWindow::readyScreen, m_preference, &Preference::readyScreen );

        m_preference_action = new QAction( tr( "Preference"), this );
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
        connect( m_repetition_level_control_action, &QAction::triggered, this, &MainWindow::onRepetitionLevelControl );
        connect( this, &MainWindow::updateCurrentRepetitionLevel, m_repetition_level_control, &RepetitionLevelControl::updateCurrentRepetitionLevel );
        connect( m_repetition_level_control , &RepetitionLevelControl::shading , m_shading_control , &ShadingControl::shading );
        emit updateCurrentRepetitionLevel();

        ui->menuTools->addAction( m_repetition_level_control_action );

        m_repetition_level_control->adjustSize();
        addDockWidget( Qt::LeftDockWidgetArea, m_repetition_level_control );
        m_repetition_level_control->close();
    }
}

void MainWindow::shadingControlInitialize()
{
    if( m_shading_control )
    {
        m_shading_control_action = new QAction( tr( "Shading Control"), this );
        connect( m_shading_control_action, &QAction::triggered, this, &MainWindow::onShadingControl );

        ui->menuTools->addAction( m_shading_control_action );

        m_shading_control->adjustSize();
        addDockWidget( Qt::LeftDockWidgetArea, m_shading_control );
        m_shading_control->close();
    }
}

void MainWindow::transferFunctionEditorInitialize()
{
    if( m_transfer_function_editor )
    {
        m_transfer_function_editor_action = new QAction( tr( "Transfer Function Editor"), this );
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
        connect( m_volume_transform_action, &QAction::triggered, this, &MainWindow::onVolumeTransform );

        ui->menuTools->addAction( m_volume_transform_action );

        m_volume_transform->adjustSize();
        addDockWidget( Qt::LeftDockWidgetArea, m_volume_transform );
        m_volume_transform->close();
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
