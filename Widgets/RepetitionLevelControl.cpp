#include "RepetitionLevelControl.h"
#include "ui_RepetitionLevelControl.h"

#include <kvs/ParticleBasedRenderer>
RepetitionLevelControl::RepetitionLevelControl(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RepetitionLevelControl),
    m_screen( nullptr ),
    m_compositor( nullptr )
{
    ui->setupUi(this);
    connect( ui->applyPBtn, &QPushButton::clicked, this, &RepetitionLevelControl::onApplyButtonClicked );
}

RepetitionLevelControl::~RepetitionLevelControl()
{
    delete ui;
}

void RepetitionLevelControl::onApplyButtonClicked()
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
                    kvs::RendererBase* particle_based_renderer = new kvs::glsl::ParticleBasedRenderer;
                    m_screen->scene()->replaceRenderer( id.second, particle_based_renderer );
                }
            }
        }
    }
    m_compositor->setRepetitionLevel( ui->nextRepetitionLevelSBox->value() );
    m_compositor->screen()->redraw();
    ui->currentRepetitionLevelLEdit->setText( QString::number( ui->nextRepetitionLevelSBox->value() ) );
}
