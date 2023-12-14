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

//デバック用(将来的にデバッグ用マクロ定義ファイルに移行する。
#define STOCHASTIC_RENDERING_WITH_HYDROGEN

PBVRGUI::PBVRGUI(kvs::qt::Application& app, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PBVRGUI),
    m_preference( this ),
    m_merge( this ),
    m_volumeTransform( this ),
    m_connect( this )
{
    ui->setupUi(this);
    setWindowTitle( "QTPBVR vX.X.X" );

    m_screen = new kvs::qt::jaea::Screen( &app );
    initialize();

    connect( ui->actionPreference, &QAction::triggered, this, &PBVRGUI::onPreference );
    connect( ui->actionMerge, &QAction::triggered, this, &PBVRGUI::onMerge );
    connect( ui->actionVolumeTransform, &QAction::triggered, this, &PBVRGUI::onVolumeTransform );
    connect( ui->actionConnectToServer, &QAction::triggered, this, &PBVRGUI::onConnect );

    m_preference.setScreen( m_screen );
    m_preference.setCompositor( m_compositor );
    m_preference.setColorMapBar( m_color_map_bar );
    m_preference.setOrientationAxis( m_orientation_axis );
    m_preference.setFPSLabel( m_fps_label );
    m_preference.setTimeStepLabel( m_time_step_label );
    m_preference.initialize();

    QWidgetAction *widgetAction = new QWidgetAction( this );
    widgetAction->setDefaultWidget( &m_timeControl );
    ui->timeControlTBar->addAction( widgetAction );

    m_merge.setScreen( m_screen );
    m_merge.setTimeControl( &m_timeControl );
    m_merge.close();
    m_merge.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, &m_merge);

    m_volumeTransform.close();
    m_volumeTransform.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, &m_volumeTransform);
    m_volumeTransform.show();
    m_volumeTransform.setScreen( m_screen );
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
    m_compositor->setRepetitionLevel(repetitions);
    m_screen->setEvent(m_compositor);

    m_color_map_bar = new kvs::ColorMapBar( m_screen );
    m_orientation_axis = new kvs::OrientationAxis( m_screen, m_screen->scene() );
    m_fps_label = new kvs::Label( m_screen );
    m_time_step_label = new kvs::Label( m_screen );

    // QGridLayout に kvs::qt::jaea::Screen を追加
    ui->screenArea->addWidget(m_screen, 0, 0, 1, 1);//コンストラクタの最後にすると表示に差異が生じる、要相談
}
