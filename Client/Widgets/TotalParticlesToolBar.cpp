#include "TotalParticlesToolBar.h"

TotalParticlesToolBar::TotalParticlesToolBar( QWidget* parent ) :
    QToolBar( parent )
{
    initialize();
}

TotalParticlesToolBar::~TotalParticlesToolBar() {}

void TotalParticlesToolBar::initialize()
{
    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout = new QHBoxLayout( containerWidget );

    m_total_particles_label = new QLabel( "Total Particles : ", this );
    m_total_particles_display = new QLabel( "NO POINT OBJECTS", this );
    m_total_particles_display->setFixedWidth( 130 );

    // Layoutにウィジェットを追加
    layout->addWidget( m_total_particles_label );
    layout->addWidget( m_total_particles_display );

    // ToolBarにウィジェットを追加
    this->addWidget( containerWidget );
}

void TotalParticlesToolBar::updateTotalParticles( int totalParticles )
{
    if( totalParticles != 0 )
    {
        m_total_particles_display->setText( QString::number( totalParticles ) );
    }
    else
    {
        m_total_particles_display->setText( "NO POINT OBJECTS" );
    }
}
