#include "pbvrgui.h"
#include "ui_pbvrgui.h"

#include <kvs/HydrogenVolumeData>
#include <kvs/CellByCellMetropolisSampling>
#include <kvs/ParticleBasedRenderer>
#include <kvs/ExternalFaces>
#include <kvs/StochasticPolygonRenderer>

PBVRGUI::PBVRGUI(kvs::qt::Application& app, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PBVRGUI)
{
    ui->setupUi(this);
    m_screen = new kvs::qt::Screen( &app );
    initialize();
}

PBVRGUI::~PBVRGUI()
{
    delete ui;
}

void PBVRGUI::initialize()
{
    kvs::Vector3ui resolution(32, 32, 32);
    kvs::StructuredVolumeObject* volume = new kvs::HydrogenVolumeData(resolution);

    const size_t repetitions = 4;
    const float step = 0.5f;
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

    m_screen->setFixedSize( 620, 620 );
    m_screen->setGeometry( 0, 0, 620, 620 );

    // ストキャスティック レンダリング コンポジタのセットアップ
    kvs::StochasticRenderingCompositor* compositor = new kvs::StochasticRenderingCompositor(m_screen->scene());
    compositor->setRepetitionLevel(repetitions);
    m_screen->setEvent(compositor);

    // QGridLayout に kvs::qt::Screen を追加
    ui->screenArea->addWidget(m_screen, 0, 0, 1, 1);
}
