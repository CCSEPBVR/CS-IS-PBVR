#include "DisplayPointSizeControl.h"
#include "ui_DisplayPointSizeControl.h"
#include "App/pbvrgui.h"
#include <kvs/ParticleBasedRenderer>
DisplayPointSizeControl::DisplayPointSizeControl(QWidget *parent, PBVRGUI *pbvr_gui) :
    QDockWidget(parent),
    ui(new Ui::DisplayPointSizeControl),
    m_pbvr_gui( pbvr_gui )
{
    ui->setupUi(this);
    connect( ui->doubleSpinBox, &QDoubleSpinBox::valueChanged, this, &DisplayPointSizeControl::onApplyButtonClicked);
}

DisplayPointSizeControl::~DisplayPointSizeControl()
{
    delete ui;
}

void DisplayPointSizeControl::onApplyButtonClicked()
{
    const int size = m_pbvr_gui->screen()->scene()->IDManager()->size();
    for( int index = 0; index < size; index++ )
    {
        auto id = m_pbvr_gui->screen()->scene()->IDManager()->id( index );
        auto* object = m_pbvr_gui->screen()->scene()->objectManager()->object( id.first );
        auto* rendererBase = m_pbvr_gui->screen()->scene()->rendererManager()->renderer( id.second );
        if( object->isVisible() && rendererBase )
        {
            if (auto* stochasticRenderer = dynamic_cast<kvs::StochasticRendererBase*>(rendererBase))
            {
                if (auto* particleRenderer = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>(stochasticRenderer))
                {
                    particleRenderer->setDisplayPointSize( ui->doubleSpinBox->value() );
                }
            }
        }
    }
    m_pbvr_gui->screen()->update();
}
