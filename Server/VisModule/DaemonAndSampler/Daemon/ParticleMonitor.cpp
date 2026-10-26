#include "ParticleMonitor.h"
//#include "Timer.h"
//#include <vismodule/Timer>

ParticleMonitor::ParticleMonitor():
    m_time_step_particle( -1 ),
    m_time_step_glyph( -1 ),
    m_time_step_pol( -1 ),
    m_time_step_pot( -1 )
{
    const char *envBuf = NULL;
    std::string particlePath;
    std::string glyphFilePath;
    std::string plotOverLineFilePath;
    std::string plotOverTimeFilePath;
    std::string visParamDir;
    std::string coordsMinMaxPath;

    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        particlePath = "./t";
        glyphFilePath = "./g";
        plotOverLineFilePath = "./p";
        plotOverTimeFilePath = "./pt";
    }
    else {
        particlePath = envBuf;
        glyphFilePath = envBuf;
        plotOverLineFilePath = envBuf;
        plotOverTimeFilePath = envBuf;
        if (particlePath[particlePath.size() - 1] != '/') {
            particlePath += "/t";
            glyphFilePath += "/g";
            plotOverLineFilePath += "/p";
            plotOverTimeFilePath += "/pt";
        }
        else {
            particlePath += "t";
            glyphFilePath += "g";
            plotOverLineFilePath += "p";
            plotOverTimeFilePath += "pt";
        }
    }

    this->setParticleFilePrefix( particlePath );
    this->setGlyphFilePrefix( glyphFilePath );
    this->setPlotOverLineFilePrefix( plotOverLineFilePath );
    this->setPlotOverTimeFilePrefix( plotOverTimeFilePath );

    envBuf = std::getenv( "VIS_PARAM_DIR" );
    if (envBuf == NULL) {
        visParamDir = "./";
    }
    else {
        visParamDir = envBuf;
        if (visParamDir[visParamDir.size() - 1] != '/') {
            visParamDir += "/";
        }
    }

    std::string statePath = visParamDir + "state.txt";
    std::string historyPath = visParamDir + "history";

    this->setParticleStatusFileName( statePath );
    m_history_file_prefix = historyPath;

    coordsMinMaxPath = particlePath + "_pfi_coords_minmax.txt";
    vismodule::File f( coordsMinMaxPath.c_str() );
    if ( f.isExisted() )
    {
        FILE* fp = NULL;
        fp = fopen( coordsMinMaxPath.c_str(), "r" );
        fscanf(
            fp,
            "%f %f %f %f %f %f",
            &m_min_object_coord[0],
            &m_min_object_coord[1],
            &m_min_object_coord[2],
            &m_max_object_coord[0],
            &m_max_object_coord[1],
            &m_max_object_coord[2]
        );
        if ( fp != NULL ) fclose( fp );
    }
    else
    {
        m_min_object_coord[0]=0.f;
        m_min_object_coord[1]=0.f;
        m_min_object_coord[2]=0.f;
        m_max_object_coord[0]=0.1;
        m_max_object_coord[1]=0.1;
        m_max_object_coord[2]=0.1;
    }
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
//    TimerStart( 6 );
    m_glyph_file.setParameterFromFile();
//    TimerStop( 6 );
//    TimerStart( 7 );
    m_glyph_file.generateGlyphObject( m_time_step_glyph, &m_glyph );
//    TimerStop( 7 );
}

bool ParticleMonitor::readPlotOverLineFile()
{
    bool result = false;
    m_plot_over_line_file.setParameterFromFile();
    result = m_plot_over_line_file.generatePOLObject( m_time_step_pol, &m_plot_over_line );
    return result;
}

bool ParticleMonitor::readPlotOverTimeFile()
{
    bool result = false;
    m_plot_over_time_file.setParameterFromFile();
    result = m_plot_over_time_file.generatePOTObject( m_time_step_pot, &m_plot_over_time );
    return result;
}

void ParticleMonitor::readParticleFile()
{
//    TimerStart( 6 );
    m_particle_file.setParameterFromFile();
//    TimerStop( 6 );
//    TimerStart( 7 );
    m_particle.clear();
    m_particle_file.generatePointObject( m_time_step_particle, &m_particle );
//    TimerStop( 7 );
}

void ParticleMonitor::readParticleHistoryFile()
{
    std::stringstream step;
    step << '_' << std::setw( 5 ) << std::setfill( '0' ) << m_time_step_particle;
    std::string history_file_name = m_history_file_prefix + step.str() + ".json";
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

void ParticleMonitor::setPlotOverTimeFilePrefix( const std::string& prefix )
{
    m_plot_over_time_file.setFilePrefix( prefix );
}

void ParticleMonitor::setParticleStatusFileName( const std::string& file_name )
{
    m_status_file.setFileName( file_name );
}

void ParticleMonitor::setParticleHistoryFileName( const std::string& file_name )
{
    m_history_file.setFileName( file_name );
}

bool ParticleMonitor::setTimeStep_particle( const vismodule::Int32 time_step )
{
    bool changed = ( m_time_step_particle != time_step ) && this->stepExisted();
    m_time_step_particle = time_step;
    return changed;
}

bool ParticleMonitor::setTimeStep_glyph( const vismodule::Int32 time_step )
{
    bool changed = ( m_time_step_glyph != time_step ) && this->stepExisted();
    m_time_step_glyph = time_step;
    return changed;
}

bool ParticleMonitor::setTimeStep_pol( const vismodule::Int32 time_step )
{
    bool changed = ( m_time_step_pol != time_step ) && this->stepExisted();
    m_time_step_pol = time_step;
    return changed;
}

bool ParticleMonitor::setTimeStep_pot( const vismodule::Int32 time_step )
{
    bool changed = ( m_time_step_pot != time_step ) && this->stepExisted();
    m_time_step_pot = time_step;
    return changed;
}

void ParticleMonitor::getParticle( vismodule::PointObject* object )
{
    (*object) = m_particle;
}

void ParticleMonitor::getGlyph( vismodule::KVSMLObjectGlyph* object )
{
    object -> setCoords( m_glyph.coords() );
    object -> setColors( m_glyph.colors() );
    object -> setDirections( m_glyph.directions() );
    object -> setSizes( m_glyph.sizes() );

}

void ParticleMonitor::getPlotOverLine( vismodule::KVSMLObjectPlotOverLine* object )
{
    object -> setXAxis(        m_plot_over_line.x_axis() );
    object -> setMask(        m_plot_over_line.mask() );
    object -> setValuesOnLine(    m_plot_over_line.values_on_line() );
}

void ParticleMonitor::getPlotOverTime( vismodule::KVSMLObjectPlotOverTime* object )
{
    object->setMask( m_plot_over_time.mask() );
    object->setValuesOnTime( m_plot_over_time.values_on_time() );
}

vismodule::Int32 ParticleMonitor::getSubpixelLevel()
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

vismodule::Int32 ParticleMonitor::getTimeStep_particle()
{
    return m_time_step_particle;
}


vismodule::Int32 ParticleMonitor::getTimeStep_glyph()
{
    return m_time_step_glyph;
}


vismodule::Int32 ParticleMonitor::getTimeStep_pol()
{
    return m_time_step_pol;
}

vismodule::Vector3f ParticleMonitor::getMinObjectCoords()
{
    return m_min_object_coord;
}

vismodule::Vector3f ParticleMonitor::getMaxObjectCoords()
{
    return m_max_object_coord;
}

ParticleMonitor::~ParticleMonitor()
{
}
