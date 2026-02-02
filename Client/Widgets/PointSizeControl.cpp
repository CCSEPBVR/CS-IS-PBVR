#include "PointSizeControl.h"
#include "ui_PointSizeControl.h"

PointSizeControl::PointSizeControl( kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::PointSizeControl )
    , m_screen( screen )
{
    ui->setupUi( this );
    connect( ui->pointSizeDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PointSizeControl::onPointSizeValueChanged );
}

PointSizeControl::~PointSizeControl()
{
    delete ui;
}

void PointSizeControl::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void PointSizeControl::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void PointSizeControl::onPointSizeValueChanged()
{
    // NOTE:この処理はPlot Over Line機能(VRを含む)で使用している始点,終点のポイントオブジェクトには影響しません
    // kvs::glsl::ParticleBasedRendererを使用して表示しているもののみ影響します
    const auto displayPointSize = ui->pointSizeDoubleSpinBox->value();
    const int size = m_screen->scene()->IDManager()->size();

    for( int index = 0; index < size; ++index )
    {
        const auto id = m_screen->scene()->IDManager()->id( index );
        auto* object = m_screen->scene()->objectManager()->object( id.first );
        auto* rendererBase = m_screen->scene()->rendererManager()->renderer( id.second );

        if( !object || !rendererBase ) { continue; }

        auto* pointObject = dynamic_cast<kvs::PointObject*>( object );
        if( !pointObject ) { continue; }

        auto* stochastic = dynamic_cast<kvs::StochasticRendererBase*>( rendererBase );
        if( !stochastic ) { continue; }

        auto* particle = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>( stochastic );
        if( !particle ) { continue; }

        particle->setDisplayPointSize( displayPointSize );
    }

    m_screen->update();
}
