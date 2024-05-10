#include "pbvrgui.h"
#include "ui_pbvrgui.h"

#include <kvs/HydrogenVolumeData>
#include <kvs/CellByCellMetropolisSampling>
#include <kvs/ParticleBasedRenderer>
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
    m_screen( nullptr ),
    m_compositor( nullptr ),
    m_color_map_bar( nullptr ),
    m_orientation_axis( nullptr ),
    m_fps_label( nullptr ),
    m_time_step_label( nullptr ),
    m_preference( this ),
    m_timeControl( this ),
    m_data_summary( this ),
    m_merge( this ),
    m_connect( this ),
    m_volumeTransform( this ),
    m_animation_controls( this ),
    m_repetition_level_control( this ),
    m_display_point_size_control( this ),
    m_render_options( this ),
    m_data_properties( this ),
    m_coordinates( this ),
    m_transfer_function_editor( this ),
    m_color_function_selector( this )
{
    ui->setupUi(this);
    setWindowTitle( "QTPBVR vX.X.X" );

    //スクリーンの初期化
    m_screen = new kvs::qt::jaea::Screen( &app );

    const size_t repetitions = 4;
    //ストキャステックレンダリングコンポジタの初期化
    m_compositor = new kvs::StochasticRenderingCompositor(m_screen->scene());
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

    //プリファレンスパネルの初期化
    m_preference.setScreen( m_screen );
    m_preference.setCompositor( m_compositor );
    m_preference.setColorMapBar( m_color_map_bar );    
    m_preference.setOrientationAxis( m_orientation_axis );
    m_preference.setFPSLabel( m_fps_label );
    m_preference.setTimeStepLabel( m_time_step_label );
    m_preference.initialize();    
    //タイムコントロールウィジェットの初期化
    QWidgetAction *timeControlWidgetAction = new QWidgetAction( this );
    timeControlWidgetAction->setDefaultWidget( &m_timeControl );
    ui->timeControlTBar->addAction( timeControlWidgetAction );
    m_timeControl.setMerge( &m_merge );
    //データサマリーウィジェットの初期化
    QWidgetAction *dataSummaryWidgetAction = new QWidgetAction( this );
    dataSummaryWidgetAction->setDefaultWidget( &m_data_summary );
    ui->dataSummaryTBar->addAction( dataSummaryWidgetAction );
    //マージパネルの初期化
    m_merge.setScreen( m_screen );
    m_merge.setPreference( &m_preference );
    m_merge.setTimeControl( &m_timeControl );
    m_merge.setDataSummary( &m_data_summary );
    m_merge.setConnect( &m_connect );
    m_merge.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, &m_merge );
    //コネクトパネルの初期化
    m_connect.setScreen( m_screen );
    m_connect.setCamera( m_screen->scene()->camera() );
    m_connect.setMerge( &m_merge );
    m_connect.setFilterInfomation( &m_data_properties );
    m_connect.setTransferFunctionEditor( &m_transfer_function_editor );

    //ボリュームトランスフォームパネルの初期化
    m_volumeTransform.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::LeftDockWidgetArea, &m_volumeTransform );
    m_volumeTransform.setScreen( m_screen );
    //アニメーションコントロールパネルの初期化
    m_animation_controls.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, &m_animation_controls );
    m_animation_controls.setScreen( m_screen );
    //リピテーションレベルコントロールパネルの初期化
    m_repetition_level_control.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::LeftDockWidgetArea, &m_repetition_level_control );
    m_repetition_level_control.setPreference( &m_preference );
    m_repetition_level_control.setScreen( m_screen );
    m_repetition_level_control.setCompositor( m_compositor );
    //ディスプレイポイントサイズコントロールパネルの初期化
    m_display_point_size_control.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::LeftDockWidgetArea, &m_display_point_size_control );
    m_display_point_size_control.setPreference( &m_preference );
    m_display_point_size_control.setScreen( m_screen );
    //データプロパティパネルの初期化
    m_data_properties.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, &m_data_properties );
    //レンダーオプションパネルの初期化
    m_render_options.close();
    m_render_options.setMerge( &m_merge );
    m_render_options.setClientMessage( m_connect.getClientMessage() );
    m_render_options.setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, &m_render_options );
    //コーディネートパネルの初期化
    m_coordinates.setMerge( &m_merge );
    m_coordinates.setClientMessage( m_connect.getClientMessage() );
    //伝達関数パネルの初期化
    m_transfer_function_editor.setColorFunctionSelector( &m_color_function_selector );
    m_transfer_function_editor.setMerge( &m_merge );
    m_transfer_function_editor.setClientMessage( m_connect.getClientMessage() );
    m_transfer_function_editor.setServerMessage( m_connect.getServerMessage() );
    m_transfer_function_editor.setReceivedMessage( m_connect.getReceivedMessage() );
    //色関数選択ウィジェット

    QWidgetAction *colorFunctionSelectorWidgetAction = new QWidgetAction( this );
    colorFunctionSelectorWidgetAction->setDefaultWidget( &m_color_function_selector );
    ui->colorFunctionSelectorTBar->addAction( colorFunctionSelectorWidgetAction );
    m_color_function_selector.setScreen( m_screen );
    m_color_function_selector.setColorMapBar( m_color_map_bar );
    m_color_function_selector.setExtendedTransferFunctionMessage( m_transfer_function_editor.getExtendedTransferFunctionMessage() );
    m_color_function_selector.populateColorFunctionLists( m_color_function_selector.getExtendedTransferFunctionMessage()->m_transfer_function_number );
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
            m_screen->scene()->reset();
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
