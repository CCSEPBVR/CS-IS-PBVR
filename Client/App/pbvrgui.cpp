#include "pbvrgui.h"
#include "ui_pbvrgui.h"

#include <kvs/HydrogenVolumeData>
#include <kvs/CellByCellMetropolisSampling>
#include <kvs/ExternalFaces>
#include <kvs/StochasticPolygonRenderer>
#include <kvs/Label>
#include <kvs/FontMetrics>
#include <QWidgetAction>
#include <kvs/PolygonImporter>
#include <kvs/WidgetBase>

PBVRGUI::PBVRGUI(kvs::qt::Application& app, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PBVRGUI),
    m_screen( new kvs::qt::jaea::Screen( &app ) ),
    m_compositor( new kvs::StochasticRenderingCompositor( m_screen->scene() ) ),
    m_color_map_bar( nullptr ),
    m_orientation_axis( nullptr ),
    m_fps_label( nullptr ),
    m_time_step_label( nullptr ),
    m_preference( this, this ),
    m_time_controller_A( this ),
    m_time_controller_B( this, &m_time_controller_A, &m_merge ),
    m_total_particles( this ),
    m_color_map_bar_selector( this, this ),
    m_merge( this, this, &m_preference ,&m_time_controller_B, &m_total_particles, &m_connect, &m_shading_controller ),
    m_connect( this, this, &m_merge, &m_data_properties, &m_render_options, &m_transfer_function_editor ),
    m_volumeTransform( this, this ),
    m_animation_controls( this, this ),
    m_repetition_level_control( this, this, &m_shading_controller ),
    m_display_point_size_control( this, this ),
    m_shading_controller( this, this ),
    m_render_options( this, &m_merge, &m_connect ),
    m_data_properties( this ),
    m_coordinates( this, &m_merge, &m_connect ),
    m_transfer_function_editor( this, &m_color_map_bar_selector ,&m_merge, &m_connect ),
    m_glyph_editor( this, &m_merge, &m_connect  ),
    m_initialize_camera_xform
    (
        kvs::Mat4(
            1, 0, 0, 0 ,
            0, 1, 0, 0 ,
            0, 0, 1, 12,
            0, 0, 0, 1
            )
        )
{
    ui->setupUi(this);
#ifdef DESKTOP_SCREEN_MODE
    setWindowTitle( "pbvr_client v" + QString( PBVR_VERSION ) + " (DESKTOP)" );
#elif OPENXR_SCREEN_MODE
    setWindowTitle( "pbvr_client v" + QString( PBVR_VERSION ) + " (OPENXR)" );
#endif

    const size_t repetitions = 4;
    m_compositor->setRepetitionLevel( repetitions );
    m_screen->setEvent(m_compositor);

    //ビューワー用ウィジェットの初期化
    m_color_map_bar = new kvs::ColorMapBar( m_screen );
    m_orientation_axis = new kvs::OrientationAxis( m_screen, m_screen->scene() );
    m_fps_label = new kvs::Label( m_screen );
    m_time_step_label = new kvs::Label( m_screen );

    // QGridLayout に kvs::qt::jaea::Screen を追加
    ui->screenArea->addWidget(m_screen, 0, 0, 1, 1);

    connect( ui->actionPreference, &QAction::triggered, this, &PBVRGUI::onPreference );
    connect( ui->actionMerge, &QAction::triggered, this, &PBVRGUI::onMerge );
    connect( ui->actionConnectToServer, &QAction::triggered, this, &PBVRGUI::onConnect );
    connect( ui->actionVolumeTransform, &QAction::triggered, this, &PBVRGUI::onVolumeTransform );
    connect( ui->actionAnimationControls, &QAction::triggered, this, &PBVRGUI::onAnimationControl );
    connect( ui->actionRepetitionLevelControl, &QAction::triggered, this, &PBVRGUI::onRepetitionLevelControl );
    connect( ui->actionDataProperties, &QAction::triggered, this, &PBVRGUI::onFilterInfomation );
    connect( ui->actionRenderOptions, &QAction::triggered, this, &PBVRGUI::onRenderOptions );
    connect( ui->actionDisplayPointSizeControl, &QAction::triggered, this, &PBVRGUI::onDisplayPointSizeControl );
    connect( ui->actionShadingControll, &QAction::triggered, this, &PBVRGUI::onShadingControl );
    connect( ui->actionCoordinates, &QAction::triggered, this, &PBVRGUI::onCoordinates );
    connect( ui->actionTransferFunctionEditor, &QAction::triggered, this, &PBVRGUI::onTransferFunctionEditor );
    setFocusPolicy(Qt::StrongFocus);    
}

PBVRGUI::~PBVRGUI()
{
    delete ui;
}

void PBVRGUI::initializePanels()
{
    //フォントディレクトリ
#ifdef Q_OS_WIN
    std::string fontDir = QDir( QCoreApplication::applicationDirPath() ).filePath( "Font/" ).toLocal8Bit().constData();
#else
    std::string fontDir = QDir( QCoreApplication::applicationDirPath() ).filePath( "Font/" ).toStdString();
#endif
    m_screen->paintDevice()->textEngine()->addFont( "SansRegular", fontDir + "NotoSans-Regular.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SansItalic", fontDir + "NotoSans-Italic.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SansBold", fontDir + "NotoSans-Bold.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SansBoldItalic", fontDir + "NotoSans-BoldItalic.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SerifRegular", fontDir + "NotoSerif-Regular.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SerifItalic", fontDir + "NotoSerif-Italic.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SerifBold", fontDir + "NotoSerif-Bold.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "SerifBoldItalic", fontDir + "NotoSerif-BoldItalic.ttf" );
    m_screen->paintDevice()->textEngine()->addFont( "Icon", fontDir + "entypo.ttf" );

    //プリファレンスパネルの初期化
    m_preference.initialize();

    //タイムコントロールウィジェットの初期化
    this->addToolBar(Qt::TopToolBarArea, &m_time_controller_A);
    this->addToolBarBreak(Qt::TopToolBarArea);
    this->addToolBar(Qt::TopToolBarArea, &m_total_particles);
    this->addToolBar(Qt::TopToolBarArea, &m_color_map_bar_selector);
    m_color_map_bar_selector.setExtendedTransferFunctionMessage( m_transfer_function_editor.getExtendedTransferFunctionMessage() );
    m_color_map_bar_selector.populateColorFunctionLists( m_color_map_bar_selector.getExtendedTransferFunctionMessage()->m_transfer_function_number );
    this->addToolBarBreak(Qt::TopToolBarArea);
    this->addToolBar(Qt::TopToolBarArea, &m_time_controller_B);    

    //マージパネルの初期化
    m_merge.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, &m_merge );

    //コネクトパネルの初期化

    //ボリュームトランスフォームパネルの初期化
    m_volumeTransform.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::LeftDockWidgetArea, &m_volumeTransform );

    //アニメーションコントロールパネルの初期化
    m_animation_controls.close();
    m_animation_controls.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, &m_animation_controls );

    //リピテーションレベルコントロールパネルの初期化
    m_repetition_level_control.close();
    m_repetition_level_control.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::LeftDockWidgetArea, &m_repetition_level_control );

    //ディスプレイポイントサイズコントロールパネルの初期化
    m_display_point_size_control.close();
    m_display_point_size_control.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::LeftDockWidgetArea, &m_display_point_size_control );    

    //シェーディングコントローラーパネルの初期化
    m_shading_controller.close();
    m_shading_controller.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::LeftDockWidgetArea, &m_shading_controller );

    //データプロパティパネルの初期化
    m_data_properties.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, &m_data_properties );

    //レンダーオプションパネルの初期化
    m_render_options.close();
    m_render_options.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, &m_render_options );

    //コーディネートパネルの初期化

    //伝達関数パネルの初期化

    // m_glyph_editor.close();
    m_glyph_editor.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, &m_glyph_editor );
}

void PBVRGUI::keyPressEvent(QKeyEvent *event)
{
    if( !m_screen->scene() ) { return; }

    //シフトキーが押されている場合(大文字)
    if( event->modifiers() == Qt::ShiftModifier )
    {
        switch( event->key() )
        {
            //アニメーションコントロール用キーバインド
        case Qt::Key_D:
            m_animation_controls.clearKeyFrame();
            break;
        case Qt::Key_M:
            m_animation_controls.playKeyFrame();
            break;
        case Qt::Key_L:
            m_animation_controls.loadKeyFrameFile();
            break;
        case Qt::Key_S:
            m_animation_controls.saveKeyFrameFile();
            break;
        default:
            break;
        }
    }
    //シフトキーが押されていない場合(小文字)
    else
    {
        switch( event->key() )
        {
        case Qt::Key_C:
            m_screen->setControlTarget( kvs::qt::jaea::Screen::ControlTarget::TargetCamera );
            break;
        case Qt::Key_L:
            m_screen->setControlTarget( kvs::qt::jaea::Screen::ControlTarget::TargetLight );
            break;
        case Qt::Key_O:
            m_screen->setControlTarget( kvs::qt::jaea::Screen::ControlTarget::TargetObject );
            break;
        case Qt::Key_Home:
            m_screen->reset();
            m_screen->update();
            break;

            //アニメーションコントロール用キーバインド
        case Qt::Key_X:
            m_animation_controls.addKeyFrameAdd( m_screen->scene()->objectManager()->xform() );
            break;
        case Qt::Key_D:
            m_animation_controls.removeLasrKeyFrame();
            break;
        default:
            break;
        }
    }
}
