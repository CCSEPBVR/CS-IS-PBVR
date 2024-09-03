#include "RepetitionLevelControl.h"
#include "ui_RepetitionLevelControl.h"
#include "App/pbvrgui.h"
#include <kvs/ParticleBasedRenderer>
RepetitionLevelControl::RepetitionLevelControl(QWidget *parent, PBVRGUI *pbvr_gui, ShadingController* shading_controller) :
    QDockWidget(parent),
    ui(new Ui::RepetitionLevelControl),
    m_pbvr_gui( pbvr_gui ),
    m_shading_controller( shading_controller )
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
                    kvs::RendererBase* particle_based_renderer = new kvs::glsl::ParticleBasedRenderer;
                    m_shading_controller->applyShading( particle_based_renderer );
                    m_pbvr_gui->screen()->scene()->replaceRenderer( id.second, particle_based_renderer );
                }
            }
        }
    }    
    m_pbvr_gui->compositor()->setRepetitionLevel( ui->nextRepetitionLevelSBox->value() );
    m_pbvr_gui->compositor()->screen()->redraw();
    ui->currentRepetitionLevelLEdit->setText( QString::number( ui->nextRepetitionLevelSBox->value() ) );
}
