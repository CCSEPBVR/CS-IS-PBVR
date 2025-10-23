#include "RepetitionLevelControl.h"
#include "ui_RepetitionLevelControl.h"

RepetitionLevelControl::RepetitionLevelControl( kvs::qt::jaea::Screen* screen,
                                                kvs::StochasticRenderingCompositor* compositor,
                                                QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::RepetitionLevelControl )
    , m_screen( screen )
    , m_compositor( compositor )
{
    initialize();
}

RepetitionLevelControl::~RepetitionLevelControl()
{
    delete ui;
}

void RepetitionLevelControl::initialize()
{
    ui->setupUi( this );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &RepetitionLevelControl::onApply );
}

void RepetitionLevelControl::updateUI()
{
    const size_t currentRepetitionLevel = m_compositor->repetitionLevel();
    ui->dispCurrentRepetitionLevel->setText( QString::number( currentRepetitionLevel ) );
}

void RepetitionLevelControl::onApply()
{
    const size_t repetitionLevel = static_cast<size_t>( ui->spinBoxNewRepetitionLevel->value() );

    const size_t size = m_screen->scene()->IDManager()->size();

    kvs::Xform initialXform
        (
            kvs::Mat4(
                1, 0, 0, 0 ,
                0, 1, 0, 0 ,
                0, 0, 1, 12,
                0, 0, 0, 1
                )
            );
    kvs::Vec3 translationOffset = m_screen->scene()->camera()->xform().translation() - initialXform.translation();

    for( int index = 0; index < size; index++ )
    {
        auto ids = m_screen->scene()->IDManager()->id( index );
        auto* object = m_screen->scene()->object( ids.first );
        auto* renderer = m_screen->scene()->renderer( ids.second );
        if( auto* particleRenderer = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>( renderer ) )
        {
            kvs::glsl::ParticleBasedRenderer* particleBasedRenderer = new kvs::glsl::ParticleBasedRenderer();
            particleBasedRenderer->enableShuffle();
            particleBasedRenderer->setTranslationOffset( translationOffset );
            particleBasedRenderer->setObjectDepth( m_screen->scene()->objectManager()->xform().scaling().z() / m_screen->scene()->camera()->xform().scaling().z() );
            emit shading( particleBasedRenderer );
            // particleBasedRenderer->disableZooming();
            m_screen->scene()->replaceRenderer( ids.second, particleBasedRenderer );
        }
    }

    m_compositor->setRepetitionLevel( repetitionLevel );
    m_compositor->screen()->redraw();
    updateUI();
}
