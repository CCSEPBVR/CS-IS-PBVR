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

//デバック用(将来的にデバッグ用マクロ定義ファイルに移行する。
#define STOCHASTIC_RENDERING_WITH_HYDROGEN

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
    m_merge( this ),
    m_volumeTransform( this ),
    m_connect( this ),
    m_timeControl( this ),
    m_animation_controls( this ),
    m_data_properties( this ),
    m_data_summary( this ),
    m_render_options( this ),
    m_repetition_level_control( this ),
    m_transfer_function_editor( this )
{
    ui->setupUi(this);
    setWindowTitle( "QTPBVR vX.X.X" );

    m_screen = new kvs::qt::jaea::Screen( &app );
    initialize();

    connect( ui->actionPreference, &QAction::triggered, this, &PBVRGUI::onPreference );
    connect( ui->actionMerge, &QAction::triggered, this, &PBVRGUI::onMerge );
    connect( ui->actionVolumeTransform, &QAction::triggered, this, &PBVRGUI::onVolumeTransform );
    connect( ui->actionConnectToServer, &QAction::triggered, this, &PBVRGUI::onConnect );
    connect( ui->actionAnimationControls, &QAction::triggered, this, &PBVRGUI::onAnimationControl );
    connect( ui->actionFilterInfomation, &QAction::triggered, this, &PBVRGUI::onFilterInfomation );
    connect( ui->actionRenderOptions, &QAction::triggered, this, &PBVRGUI::onRenderOptions );
    connect( ui->actionRepetitionLevelControl, &QAction::triggered, this, &PBVRGUI::onRepetitionLevelControl );
    connect( ui->actionTransferFunctionEditor, &QAction::triggered, this, &PBVRGUI::onTransferFunctionEditor );

    m_preference.setScreen( m_screen );
    m_preference.setCompositor( m_compositor );
    m_preference.setColorMapBar( m_color_map_bar );
    m_preference.setOrientationAxis( m_orientation_axis );
    m_preference.setFPSLabel( m_fps_label );
    m_preference.setTimeStepLabel( m_time_step_label );
    m_preference.initialize();

    QWidgetAction *timeControlWidgetAction = new QWidgetAction( this );
    timeControlWidgetAction->setDefaultWidget( &m_timeControl );
    ui->timeControlTBar->addAction( timeControlWidgetAction );

    QWidgetAction *dataSummaryWidgetAction = new QWidgetAction( this );
    dataSummaryWidgetAction->setDefaultWidget( &m_data_summary );
    ui->dataSummaryTBar->addAction( dataSummaryWidgetAction );

    m_merge.setScreen( m_screen );
    m_merge.setTimeControl( &m_timeControl );
    m_merge.setPreference( &m_preference );
    m_merge.setConnect( &m_connect );
    m_merge.setDataSummary( &m_data_summary );
    m_merge.close();
    m_merge.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, &m_merge);

    m_volumeTransform.close();
    m_volumeTransform.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, &m_volumeTransform);
    m_volumeTransform.show();
    m_volumeTransform.setScreen( m_screen );

    m_animation_controls.close();
    m_animation_controls.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, &m_animation_controls);
    m_animation_controls.show();
    m_animation_controls.setScreen( m_screen );

    m_data_properties.close();
    m_data_properties.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, &m_data_properties);
    m_data_properties.show();

    m_connect.setScreen( m_screen );
    m_connect.setCamera( m_screen->scene()->camera() );
    m_connect.setMerge( &m_merge );
    m_connect.setFilterInfomation( &m_data_properties );
    m_connect.setTransferFunctionEditor( &m_transfer_function_editor );

    m_render_options.close();
    m_render_options.setClientMessage( m_connect.getClientMessage() );
    m_render_options.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, &m_render_options);

    m_repetition_level_control.close();
    m_repetition_level_control.setScreen( m_screen );
    m_repetition_level_control.setCompositor( m_compositor );
    m_repetition_level_control.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, &m_repetition_level_control);
    m_repetition_level_control.setScreen( m_screen );

    m_transfer_function_editor.setClientMessage( m_connect.getClientMessage() );
    m_transfer_function_editor.setServerMessage( m_connect.getServerMessage() );
    m_transfer_function_editor.setReceivedMessage( m_connect.getReceivedMessage() );

    setFocusPolicy(Qt::StrongFocus);
}

PBVRGUI::~PBVRGUI()
{
    delete ui;
}

void PBVRGUI::initialize()
{
    const size_t repetitions = 4;

    // ストキャスティック レンダリング コンポジタのセットアップ
    m_compositor = new kvs::StochasticRenderingCompositor(m_screen->scene());
    m_compositor->setRepetitionLevel( repetitions );
    m_screen->setEvent(m_compositor);

    m_color_map_bar = new kvs::ColorMapBar( m_screen );
    m_orientation_axis = new kvs::OrientationAxis( m_screen, m_screen->scene() );
    m_fps_label = new kvs::Label( m_screen );
    m_time_step_label = new kvs::Label( m_screen );

    // QGridLayout に kvs::qt::jaea::Screen を追加
    ui->screenArea->addWidget(m_screen, 0, 0, 1, 1);//コンストラクタの最後にすると表示に差異が生じる、要相談
}

void PBVRGUI::keyPressEvent(QKeyEvent *event)
{
    if( !m_screen->scene() ) { return; }

    //シフトキーが押されている場合(大文字)
    if( event->modifiers() == Qt::ShiftModifier )
    {
        switch( event->key() )
        {
            //for AnimationControls
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

            //for AnimationControls
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
