#include "pbvrgui.h"
#include "ui_pbvrgui.h"

#include <kvs/HydrogenVolumeData>
#include <kvs/CellByCellMetropolisSampling>
#include <kvs/ParticleBasedRenderer>
#include <kvs/ExternalFaces>
#include <kvs/StochasticPolygonRenderer>

//デバック用(将来的にデバッグ用マクロ定義ファイルに移行する。
#define STOCHASTIC_RENDERING_WITH_HYDROGEN

PBVRGUI::PBVRGUI(kvs::qt::Application& app, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PBVRGUI),
    m_preference( this ),
    m_volumeTransform( this )
{
    ui->setupUi(this);
    setWindowTitle( "QTPBVR vX.X.X" );

    m_screen = new kvs::qt::Screen( &app );
    initialize();

    connect( ui->actionPreference, &QAction::triggered, this, &PBVRGUI::onPreference );
    connect( ui->actionVolumeTransform, &QAction::triggered, this, &PBVRGUI::onVolumeTransform );

    m_preference.setScreen( m_screen );
    m_preference.setColorMapBar( m_color_map_bar );
    m_preference.setOrientationAxis( m_orientation_axis );
    m_preference.initialize();

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
    const float step = 0.5f;


#ifdef STOCHASTIC_RENDERING_WITH_HYDROGEN
    kvs::Vector3ui resolution(32, 32, 32);
    kvs::StructuredVolumeObject* volume = new kvs::HydrogenVolumeData(resolution);

    const kvs::TransferFunction tfunc(256);
    kvs::PointObject* object = new kvs::CellByCellMetropolisSampling(volume, repetitions, step, tfunc);

    kvs::glsl::ParticleBasedRenderer* renderer = new kvs::glsl::ParticleBasedRenderer();
    renderer->setRepetitionLevel(repetitions);
    renderer->enableLODControl();

    kvs::PolygonObject* polygonObject = new kvs::ExternalFaces(volume);
    polygonObject->setColor(kvs::RGBColor::White());
    polygonObject->setOpacity(32);
    kvs::StochasticPolygonRenderer* polygonRenderer = new kvs::StochasticPolygonRenderer();
    delete volume;

    // kvs::qt::Screen にオブジェクトを登録
    m_screen->registerObject(object, renderer);
    m_screen->registerObject(polygonObject, polygonRenderer);
#endif
    m_screen->setFixedSize( 620, 620 );
    m_screen->setGeometry( 0, 0, 620, 620 );

    // ストキャスティック レンダリング コンポジタのセットアップ
    compositor = new kvs::StochasticRenderingCompositor(m_screen->scene());
    compositor->setRepetitionLevel(repetitions);
    m_screen->setEvent(compositor);

    m_color_map_bar = new kvs::ColorMapBar( m_screen );
    m_orientation_axis = new kvs::OrientationAxis( m_screen, m_screen->scene() );

    // QGridLayout に kvs::qt::Screen を追加
    ui->screenArea->addWidget(m_screen, 0, 0, 1, 1);
}
