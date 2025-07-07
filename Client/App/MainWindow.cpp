#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow( kvs::qt::Application& app, QWidget *parent )
    : QMainWindow(parent)
    , ui( new Ui::MainWindow )
    , m_screen( new kvs::qt::jaea::Screen( &app ) )
    , m_compositor( new kvs::StochasticRenderingCompositor( m_screen->scene() ) )
    , m_color_map_bar( nullptr )                    // kvs::Sceneの準備が終わってないので初期化子リストで生成してはいけない。
    , m_orientation_axis( nullptr )                 // kvs::Sceneの準備が終わってないので初期化子リストで生成してはいけない。
    , m_fps_label( nullptr )                        // kvs::Sceneの準備が終わってないので初期化子リストで生成してはいけない。
    , m_time_step_label( nullptr )                  // kvs::Sceneの準備が終わってないので初期化子リストで生成してはいけない。
    , m_color_map_bar_selector_tool_bar( nullptr )  // kvs::Sceneの準備が終わってないので初期化子リストで生成してはいけない。
    , m_play_back_control_tool_bar( new PlayBackControlToolBar( this ) )
    , m_time_step_control_tool_bar( new TimeStepControlToolBar( this ) )
    , m_total_particle_tool_bar( new TotalParticlesToolBar( this ) )
    , m_animation_control( new AnimationControl( m_screen, this ) )
    , m_connect( new Connect( m_screen, this ) )
    , m_glyph_editor( new GlyphEditor( this ) )
    , m_object_editor( new ObjectEditor( m_screen, m_connect, this ) )
    , m_plot_over_line( new PlotOverLine( m_screen, this ) )
    , m_point_size_control( new PointSizeControl( m_screen, this ) )
    , m_preference( nullptr )                       // kvs::Sceneの準備が終わってないので初期化子リストで生成してはいけない。
    , m_repetition_level_control( new RepetitionLevelControl( m_screen, m_compositor, this ) )
    , m_shading_control( new ShadingControl( m_screen, this ) )
    , m_transfer_function_editor( new TransferFunctionEditor( this ) )
    , m_volume_transform( new VolumeTransform( m_screen, this ) )
{
    initialize();
}

MainWindow::~MainWindow()
{
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
    // UIのセットアップ
    ui->setupUi( this );
    setWindowTitle( QString("pbvr_client - %1").arg( GIT_BRANCH_NAME ) );

    m_compositor->setRepetitionLevel( 4 ); // コンポジターのリピートレベルを設定 初期値:4
    m_screen->setEvent( m_compositor );

    m_screen->setFixedSize( 620, 620 );
    ui->screenArea->addWidget( m_screen );

    // プリファレンス
    m_color_map_bar = new kvs::ColorMapBar( m_screen );
    m_orientation_axis = new kvs::OrientationAxis( m_screen, m_screen->scene() );
    m_fps_label = new kvs::Label( m_screen );
    m_time_step_label = new kvs::Label( m_screen );
    m_preference = new Preference( m_screen, m_compositor, m_color_map_bar, m_orientation_axis, m_fps_label, m_time_step_label, this );
    m_preference->readyScreen();

    m_color_map_bar_selector_tool_bar = new ColorMapSelectorToolBar( m_screen, m_color_map_bar, this );

    setDockOptions( QMainWindow::AnimatedDocks );
    m_animation_control->adjustSize();
    m_glyph_editor->adjustSize();
    m_object_editor->adjustSize();
    m_plot_over_line->adjustSize();
    m_point_size_control->adjustSize();
    m_repetition_level_control->adjustSize();
    m_shading_control->adjustSize();
    m_volume_transform->adjustSize();

    this->addToolBar( Qt::TopToolBarArea, m_time_step_control_tool_bar );
    this->addToolBarBreak( Qt::TopToolBarArea );
    this->addToolBar( Qt::TopToolBarArea, m_total_particle_tool_bar );
    this->addToolBar( Qt::TopToolBarArea, m_color_map_bar_selector_tool_bar );
    this->addToolBarBreak( Qt::TopToolBarArea );
    this->addToolBar( Qt::TopToolBarArea, m_play_back_control_tool_bar );

    addDockWidget( Qt::LeftDockWidgetArea   , m_animation_control );
    addDockWidget( Qt::LeftDockWidgetArea   , m_object_editor );
    addDockWidget( Qt::RightDockWidgetArea  , m_plot_over_line );
    addDockWidget( Qt::RightDockWidgetArea  , m_point_size_control );
    m_repetition_level_control->updateUI();
    addDockWidget( Qt::RightDockWidgetArea  , m_repetition_level_control );
    addDockWidget( Qt::RightDockWidgetArea  , m_shading_control );
    addDockWidget( Qt::LeftDockWidgetArea   , m_volume_transform );

    // アクションコネクト
    connect( ui->actionAnimationControl         , &QAction::triggered                                           , this                              , &MainWindow::onAnimationControl );
    connect( ui->actionConnect                  , &QAction::triggered                                           , this                              , &MainWindow::onConnect );
    connect( ui->actionGlyphEditor              , &QAction::triggered                                           , this                              , &MainWindow::onGlyphEditor );
    connect( ui->actionObjectEditor             , &QAction::triggered                                           , this                              , &MainWindow::onObjectEditor );
    connect( ui->actionPlotOverLine             , &QAction::triggered                                           , this                              , &MainWindow::onPlotOverLine );
    connect( ui->actionPointSizeControl         , &QAction::triggered                                           , this                              , &MainWindow::onPointSizeControl );
    connect( ui->actionPreference               , &QAction::triggered                                           , this                              , &MainWindow::onPreference );
    connect( ui->actionRepetitionLevelControl   , &QAction::triggered                                           , this                              , &MainWindow::onRepetitionLevelControl );
    connect( ui->actionShadingControl           , &QAction::triggered                                           , this                              , &MainWindow::onShadingControl );
    connect( ui->actionTransferFunctionEditor   , &QAction::triggered                                           , this                              , &MainWindow::onTransferFunctionEditor );
    connect( ui->actionVolumeTransform          , &QAction::triggered                                           , this                              , &MainWindow::onVolumeTransform );

    // カラーマップセレクター(ツールバー)

    // プレイバックコントロール(ツールバー)
    connect( m_play_back_control_tool_bar       , &PlayBackControlToolBar::fisrtTimeStep                        , m_time_step_control_tool_bar      , &TimeStepControlToolBar::fisrtTimeStep );
    connect( m_play_back_control_tool_bar       , &PlayBackControlToolBar::previousTimeStep                     , m_time_step_control_tool_bar      , &TimeStepControlToolBar::previousTimeStep );
    connect( m_play_back_control_tool_bar       , &PlayBackControlToolBar::reverseTimeStep                      , m_time_step_control_tool_bar      , &TimeStepControlToolBar::reverseTimeStep );
    connect( m_play_back_control_tool_bar       , &PlayBackControlToolBar::playTimeStep                         , m_time_step_control_tool_bar      , &TimeStepControlToolBar::playTimeStep );
    connect( m_play_back_control_tool_bar       , &PlayBackControlToolBar::nextTimeStep                         , m_time_step_control_tool_bar      , &TimeStepControlToolBar::nextTimeStep );
    connect( m_play_back_control_tool_bar       , &PlayBackControlToolBar::lastTimeStep                         , m_time_step_control_tool_bar      , &TimeStepControlToolBar::lastTimeStep );
    connect( m_play_back_control_tool_bar       , &PlayBackControlToolBar::keepLastTimeStep                     , m_time_step_control_tool_bar      , &TimeStepControlToolBar::keepLastTimeStep );
    connect( m_play_back_control_tool_bar       , &PlayBackControlToolBar::jumpTimeStep                         , m_time_step_control_tool_bar      , &TimeStepControlToolBar::jumpTimeStep );
    connect( m_play_back_control_tool_bar       , &PlayBackControlToolBar::loopMode                             , m_time_step_control_tool_bar      , &TimeStepControlToolBar::loopMode );

    // タイムステップコントロール(ツールバー)
    connect( m_time_step_control_tool_bar       , &TimeStepControlToolBar::requestMerge                         , m_object_editor                   , &ObjectEditor::doMerge );
    connect( m_time_step_control_tool_bar       , &TimeStepControlToolBar::doneMerge                            , m_play_back_control_tool_bar      , &PlayBackControlToolBar::doneMerge );

    // トータルパーティクル(ツールバー)

    // アニメーションコントロール
    connect( m_screen                           , &kvs::qt::jaea::Screen::addKeyFrameAdd                        , m_animation_control               , &AnimationControl::addKeyFrameAdd );
    connect( m_screen                           , &kvs::qt::jaea::Screen::removeLastKeyFrame                    , m_animation_control               , &AnimationControl::removeLastKeyFrame );
    connect( m_screen                           , &kvs::qt::jaea::Screen::clearKeyFrame                         , m_animation_control               , &AnimationControl::clearKeyFrame );
    connect( m_screen                           , &kvs::qt::jaea::Screen::playKeyFrame                          , m_animation_control               , &AnimationControl::playKeyFrame );
    connect( m_screen                           , &kvs::qt::jaea::Screen::loadKeyFrameFile                      , m_animation_control               , &AnimationControl::loadKeyFrameFile );
    connect( m_screen                           , &kvs::qt::jaea::Screen::saveKeyFrameFile                      , m_animation_control               , &AnimationControl::saveKeyFrameFile );
    connect( m_screen                           , &kvs::qt::jaea::Screen::updatePointsTranslation               , m_plot_over_line                  , &PlotOverLine::updatePointsTranslation );

    // コネクト
    connect( m_connect                          , &Connect::updateNumberOfVector                                , m_glyph_editor                    , &GlyphEditor::updateNumberOfVector                            , Qt::QueuedConnection );

    connect( m_connect                          , &Connect::serverPointObjectCS                                 , m_object_editor                   , &ObjectEditor::serverPointObjectCS );
    connect( m_connect                          , &Connect::serverGlyphObjectCS                                 , m_object_editor                   , &ObjectEditor::serverGlyphObjectCS );
    connect( m_connect                          , &Connect::serverPointObjectIS                                 , m_object_editor                   , &ObjectEditor::serverPointObjectIS                            , Qt::QueuedConnection );
    connect( m_connect                          , &Connect::serverGlyphObjectIS                                 , m_object_editor                   , &ObjectEditor::serverGlyphObjectIS                            , Qt::QueuedConnection );
    connect( m_connect                          , &Connect::insituObjectActive                                  , m_object_editor                   , &ObjectEditor::insituObjectActive                             , Qt::QueuedConnection );
    connect( m_connect                          , &Connect::updateInSituTimeStep                                , m_object_editor                   , &ObjectEditor::updateInSituTimeStep                           , Qt::QueuedConnection );

    connect( m_connect                          , &Connect::setPlotData                                         , m_plot_over_line                  , &PlotOverLine::setPlotData                                    , Qt::QueuedConnection );
    connect( m_connect                          , &Connect::updateNumberOfVector                                , m_plot_over_line                  , &PlotOverLine::updateNumberOfVector                           , Qt::QueuedConnection );

    connect( m_connect                          , &Connect::importTransferFunctionFromFile                      , m_transfer_function_editor        , &TransferFunctionEditor::importTransferFunctionFromFile );
    connect( m_connect                          , &Connect::importTransferFunctionFromServer                    , m_transfer_function_editor        , &TransferFunctionEditor::importTransferFunctionFromServer );
    connect( m_connect                          , &Connect::applyTransferFunction                               , m_transfer_function_editor        , &TransferFunctionEditor::applyTransferFunction );
    connect( m_connect                          , &Connect::updateTransferFunctionRangeAndView                  , m_transfer_function_editor        , &TransferFunctionEditor::updateTransferFunctionRangeAndView   , Qt::QueuedConnection );

    // グリフエディター
    connect( m_glyph_editor                     , &GlyphEditor::updateGlyphParameterClientMessage               , m_connect                         , &Connect::updateGlyphParameterClientMessage );
    connect( m_glyph_editor                     , &GlyphEditor::requestReplaceServerGlyphObject                 , m_object_editor                   , &ObjectEditor::requestReplaceServerGlyphObject );

    // オブジェクトエディター
    connect( m_object_editor                    , &ObjectEditor::updateTotalTimeStepRange                       , m_time_step_control_tool_bar      , &TimeStepControlToolBar::updateTotalTimeStepRange );
    connect( m_object_editor                    , &ObjectEditor::mergingFinish                                  , m_time_step_control_tool_bar      , &TimeStepControlToolBar::mergeFinish );
    connect( m_object_editor                    , &ObjectEditor::noItems                                        , m_time_step_control_tool_bar      , &TimeStepControlToolBar::noItems );
    connect( m_object_editor                    , &ObjectEditor::updateInSituObjectMinMaxTimeStep               , m_time_step_control_tool_bar      , &TimeStepControlToolBar::updateInSituObjectMinMaxTimeStep );

    connect( m_object_editor                    , &ObjectEditor::mergingFinish                                  , m_preference                      , &Preference::mergingFinish );
    connect( m_object_editor                    , &ObjectEditor::updateFocus                                    , m_plot_over_line                  , &PlotOverLine::updateFocus );
    connect( m_object_editor                    , &ObjectEditor::updatePointsTranslation                        , m_plot_over_line                  , &PlotOverLine::updatePointsTranslation );
    connect( m_object_editor                    , &ObjectEditor::shading                                        , m_shading_control                 , &ShadingControl::shading );
    connect( m_object_editor                    , &ObjectEditor::updateTotalParticles                           , m_total_particle_tool_bar         , &TotalParticlesToolBar::updateTotalParticles );

    connect( m_object_editor                    , &ObjectEditor::updateRenderParameterClientMessage             , m_connect                         , &Connect::updateRenderParameterClientMessage );
    connect( m_object_editor                    , &ObjectEditor::updateCoordinateParameterClientMessage         , m_connect                         , &Connect::updateCoordinateParameterClientMessage );

    // プロットオーバーライン
    connect( m_plot_over_line                   , &PlotOverLine::updatePlotOverLineParameterClientMessage       , m_connect                         , &Connect::updatePlotOverLineParameterClientMessage );
    connect( m_plot_over_line                   , &PlotOverLine::requestReplaceServerPointObject                , m_object_editor                   , &ObjectEditor::requestReplaceServerPointObject );

    // レぺテーションレベルコントロール
    connect( m_repetition_level_control         , &RepetitionLevelControl::shading                              , m_shading_control                 , &ShadingControl::shading );

    // トランスファーファンクション
    connect( m_transfer_function_editor         , &TransferFunctionEditor::updateColorMapBar                    , m_color_map_bar_selector_tool_bar , &ColorMapSelectorToolBar::updateColorMapBar );
    connect( m_transfer_function_editor         , &TransferFunctionEditor::failedTransferFunctionImport         , m_connect                         , &Connect::failedTransferFunctionImport );
    connect( m_transfer_function_editor         , &TransferFunctionEditor::successTransferFunctionImport        , m_connect                         , &Connect::successTransferFunctionImport );
    connect( m_transfer_function_editor         , &TransferFunctionEditor::updateTransferFunctionClientMessage  , m_connect                         , &Connect::updateTransferFunctionClientMessage );
    connect( m_transfer_function_editor         , &TransferFunctionEditor::requestReplaceServerPointObject      , m_object_editor                   , &ObjectEditor::requestReplaceServerPointObject );

    // ボリュームトランスフォーム
    connect( m_volume_transform                 , &VolumeTransform::updatePointsTranslation                     , m_plot_over_line                  , &PlotOverLine::updatePointsTranslation );

    m_animation_control->hide();
    m_glyph_editor->hide();
    // m_object_editor->hide();
    m_plot_over_line->hide();
    m_point_size_control->hide();
    m_repetition_level_control->hide();
    m_shading_control->hide();
    m_volume_transform->hide();

#ifdef OPENXR_SCREEN
    m_vr_listener = new VRHandControllerListener( m_screen );
    m_screen->addEvent( m_vr_listener );
    connect( m_vr_listener, &VRHandControllerListener::drawPlotOverLine, m_plot_over_line, &PlotOverLine::drawPlotOverLine );
    connect( m_vr_listener, &VRHandControllerListener::showHidePlotOverLine, m_plot_over_line, &PlotOverLine::showHidePlotOverLine );
#endif

    this->show();

    std::string shaderDir = Utils::toNativePath( QDir( QCoreApplication::applicationDirPath() ).filePath( "Shader" ) );
    kvs::ShaderSource::AddSearchPath( shaderDir );

    // Fontの追加はScreenが表示されてからでないとクラッシュしてしまうので注意してください。
    std::string fontDir = Utils::toNativePath( QDir( QCoreApplication::applicationDirPath() ).filePath( "Font/" ) );
    m_screen->paintDevice()->textEngine()->addFont( "SansRegular"       , fontDir + "NotoSans-Regular.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SansItalic"        , fontDir + "NotoSans-Italic.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SansBold"          , fontDir + "NotoSans-Bold.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SansBoldItalic"    , fontDir + "NotoSans-BoldItalic.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SerifRegular"      , fontDir + "NotoSerif-Regular.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SerifItalic"       , fontDir + "NotoSerif-Italic.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SerifBold"         , fontDir + "NotoSerif-Bold.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SerifBoldItalic"   , fontDir + "NotoSerif-BoldItalic.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "Icon"              , fontDir + "entypo.ttf" );
}
