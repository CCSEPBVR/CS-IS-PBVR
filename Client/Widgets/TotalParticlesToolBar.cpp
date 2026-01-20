#include "TotalParticlesToolBar.h"

TotalParticlesToolBar::TotalParticlesToolBar( kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QToolBar( parent )
    , m_screen( screen )
{
    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout = new QHBoxLayout( containerWidget );

    m_total_particles_label = new QLabel( "Total Particles : ", this );
    m_total_particles_display = new QLabel( k_no_point_objects_text, this );
    m_total_particles_display->setFixedWidth( 130 );

    layout->addWidget( m_total_particles_label );
    layout->addWidget( m_total_particles_display );

    this->addWidget( containerWidget );
    this->setMovable( false );
}

TotalParticlesToolBar::~TotalParticlesToolBar() {}

void TotalParticlesToolBar::onUpdateTotalParticles()
{
    const int size = m_screen->scene()->IDManager()->size();
    int totalParticles = 0;

    for( int index = 0; index < size; ++index )
    {
        auto id = m_screen->scene()->IDManager()->id( index );
        auto* object = m_screen->scene()->objectManager()->object( id.first );
        auto* rendererBase = m_screen->scene()->rendererManager()->renderer( id.second );

        if( !object || !rendererBase ) continue;
        if( !object->isVisible() ) continue;

        auto* pointObject = dynamic_cast<kvs::PointObject*>( object );
        if( !pointObject ) continue;

        auto* stochastic = dynamic_cast<kvs::StochasticRendererBase*>( rendererBase );
        if( !stochastic ) continue;

        auto* particle = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>( stochastic );
        if( !particle ) continue;

        totalParticles += pointObject->numberOfVertices();
    }

    m_total_particles_display->setText( totalParticles != 0 ? QString::number( totalParticles ) : k_no_point_objects_text );
}
