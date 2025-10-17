#include "PointSizeControl.h"
#include "ui_PointSizeControl.h"

PointSizeControl::PointSizeControl( kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::PointSizeControl)
    , m_screen( screen )
{
    initialize();
}

PointSizeControl::~PointSizeControl()
{
    delete ui;
}

void PointSizeControl::initialize()
{
    ui->setupUi(this);
    connect( ui->pointSizeDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PointSizeControl::onPointSizeValueChanged );
}

// Note: この処理はPlot Over Line用（VRを含む）の始点・終点には影響しません。kvs::glsl::ParticleBasedRendererを使用して表示しているオブジェクトにのみ影響します。
void PointSizeControl::onPointSizeValueChanged()
{
    const int size = m_screen->scene()->IDManager()->size();
    for( int index = 0; index < size; index++ )
    {
        auto ids = m_screen->scene()->IDManager()->id( index );
        auto* objectBase = m_screen->scene()->objectManager()->object( ids.first );
        auto* rendererBase = m_screen->scene()->rendererManager()->renderer( ids.second );
        if( rendererBase )
        {
            if( auto* stochasticRenderer = dynamic_cast<kvs::StochasticRendererBase*>( rendererBase ) )
            {
                if( auto* particleRenderer = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>( stochasticRenderer ) )
                {
                    particleRenderer->setDisplayPointSize( ui->pointSizeDoubleSpinBox->value() );
                }
            }
        }
    }
    m_screen->update();
}
