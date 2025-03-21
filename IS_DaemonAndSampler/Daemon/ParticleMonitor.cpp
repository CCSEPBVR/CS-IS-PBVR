#include "ParticleMonitor.h"
#include "Timer.h"

ParticleMonitor::ParticleMonitor():
    m_particle_file(),
    m_status_file()
{
}

ParticleMonitor::ParticleMonitor( const std::string& particle_file_prefix,
                                  const std::string& particle_status_file_name,
                                  const std::string& particle_history_file_prefix ):
    m_time_step_particle( -1 ),
    m_time_step_glyph( -1 ),
    m_time_step_pol( -1 ),
    m_history_file_prefix( particle_history_file_prefix )
{
    this->setParticleFilePrefix( particle_file_prefix );
    this->setParticleStatusFileName( particle_status_file_name );
}

ParticleMonitor::ParticleMonitor( const std::string& particle_file_prefix,
                                  const std::string& glyph_file_prefix,
                                  const std::string& plot_over_line_file_prefix,
                                  const std::string& particle_status_file_name,
                                  const std::string& particle_history_file_prefix ):
    m_time_step_particle( -1 ),
    m_time_step_glyph( -1 ),
    m_time_step_pol( -1 ),
    m_history_file_prefix( particle_history_file_prefix )
{
    this->setParticleFilePrefix( particle_file_prefix );
    this->setGlyphFilePrefix( glyph_file_prefix );
    this->setPlotOverLineFilePrefix( plot_over_line_file_prefix );
    this->setParticleStatusFileName( particle_status_file_name );
}
void ParticleMonitor::check()
{
    m_status_file.read();
}

bool ParticleMonitor::stepExisted()
{
    bool existed = ( m_status_file.getStatus() != ParticleStatusFile::NO_STATUS_FILE )
                && ( m_status_file.getStatus() != ParticleStatusFile::NO_STEP );
    return existed;
}

bool ParticleMonitor::findGlyphFile()
{
    m_glyph_file.setParameterFromFile();
    if(m_glyph_file.subVolumeNumber()> 0 ) return true;
    else return false;
}

void ParticleMonitor::readGlyphFile()
{
    TimerStart( 6 );
    m_glyph_file.setParameterFromFile();
    TimerStop( 6 );
    TimerStart( 7 );
    m_glyph_file.generateGlyphObject( m_time_step_glyph, &m_glyph );
    TimerStop( 7 );
}

void ParticleMonitor::readPlotOverLineFile()
{
    TimerStart( 6 );
    m_plot_over_line_file.setParameterFromFile();
    TimerStop( 6 );
    TimerStart( 7 );
    m_plot_over_line_file.generatePOLObject( m_time_step_pol, &m_plot_over_line );
    TimerStop( 7 );
}


void ParticleMonitor::readParticleFile()
{
    TimerStart( 6 );
    m_particle_file.setParameterFromFile();
    TimerStop( 6 );
    TimerStart( 7 );
    m_particle.clear();
    m_particle_file.generatePointObject( m_time_step_particle, &m_particle );
    TimerStop( 7 );
}

void ParticleMonitor::readParticleHistoryFile()
{
    std::stringstream step;
    step << '_' << std::setw( 5 ) << std::setfill( '0' ) << m_time_step_particle;
    std::string history_file_name = m_history_file_prefix + step.str() + ".txt";
    this->setParticleHistoryFileName( history_file_name );
    m_history_file.read();
}

void ParticleMonitor::setParticleFilePrefix( const std::string& prefix )
{
    m_particle_file.setFilePrefix( prefix );
}

void ParticleMonitor::setGlyphFilePrefix( const std::string& prefix )
{
    m_glyph_file.setFilePrefix( prefix );
}

void ParticleMonitor::setPlotOverLineFilePrefix( const std::string& prefix )
{
    m_plot_over_line_file.setFilePrefix( prefix );
}

void ParticleMonitor::setParticleStatusFileName( const std::string& file_name )
{
    m_status_file.setFileName( file_name );
}

void ParticleMonitor::setParticleHistoryFileName( const std::string& file_name )
{
    m_history_file.setFileName( file_name );
}

bool ParticleMonitor::setTimeStep_particle( const kvs::Int32 time_step )
{
    bool changed = ( m_time_step_particle != time_step ) && this->stepExisted();
    m_time_step_particle = time_step;
    return changed;
}

bool ParticleMonitor::setTimeStep_glyph( const kvs::Int32 time_step )
{
    bool changed = ( m_time_step_glyph != time_step ) && this->stepExisted();
    m_time_step_glyph = time_step;
    return changed;
}

bool ParticleMonitor::setTimeStep_pol( const kvs::Int32 time_step )
{
    bool changed = ( m_time_step_pol != time_step ) && this->stepExisted();
    m_time_step_pol = time_step;
    return changed;
}



void ParticleMonitor::getParticle( pbvr::PointObject* object )
{
    (*object) = m_particle;
}

void ParticleMonitor::getGlyph( kvs::KVSMLObjectGlyph* object )
{
    object -> setCoords( m_glyph.coords() );
    object -> setColors( m_glyph.colors() );
    object -> setDirections( m_glyph.directions() );
    object -> setSizes( m_glyph.sizes() );

}

void ParticleMonitor::getPlotOverLine( kvs::KVSMLObjectPlotOverLine* object )
{
    object -> setXAxis(        m_plot_over_line.x_axis() );
    object -> setMask(        m_plot_over_line.mask() );
    object -> setValuesOnLine(    m_plot_over_line.values_on_line() );
}

kvs::Int32 ParticleMonitor::getSubpixelLevel()
{
    return m_particle.size( 0 );
}

ParticleStatusFile& ParticleMonitor::particleStatusFile()
{
    return m_status_file;
}

ParticleHistoryFile& ParticleMonitor::particleHistoryFile()
{
    return m_history_file;
}

GlyphFile& ParticleMonitor::glyphFile()
{
    return m_glyph_file;
}

PlotOverLineFile& ParticleMonitor::plotOverLineFile()
{
    return m_plot_over_line_file;
}

bool ParticleMonitor::statusFileChanged()
{
    return !this->checking_status_file();
}

kvs::Int32 ParticleMonitor::getTimeStep_particle()
{
    return m_time_step_particle;
}


kvs::Int32 ParticleMonitor::getTimeStep_glyph()
{
    return m_time_step_glyph;
}


kvs::Int32 ParticleMonitor::getTimeStep_pol()
{
    return m_time_step_pol;
}

ParticleMonitor::~ParticleMonitor()
{
}
