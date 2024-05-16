#include "DisplayPointSizeControl.h"
#include "ui_DisplayPointSizeControl.h"

#include <kvs/ParticleBasedRenderer>
DisplayPointSizeControl::DisplayPointSizeControl(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DisplayPointSizeControl),
    m_screen( nullptr )
{
    ui->setupUi(this);
//    connect( ui->pushButton, &QPushButton::clicked, this, &DisplayPointSizeControl::onApplyButtonClicked );
    connect( ui->doubleSpinBox, &QDoubleSpinBox::valueChanged, this, &DisplayPointSizeControl::onApplyButtonClicked);
}

DisplayPointSizeControl::~DisplayPointSizeControl()
{
    delete ui;
}

void DisplayPointSizeControl::onApplyButtonClicked()
{
    const int size = m_screen->scene()->IDManager()->size();
    for( int index = 0; index < size; index++ )
    {
        auto id = m_screen->scene()->IDManager()->id( index );
        auto* object = m_screen->scene()->objectManager()->object( id.first );
        auto* rendererBase = m_screen->scene()->rendererManager()->renderer( id.second );
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
    m_screen->update();
}
