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
    m_time_step( -1 ),
    m_history_file_prefix( particle_history_file_prefix )
{
    this->setParticleFilePrefix( particle_file_prefix );
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

void ParticleMonitor::readParticleFile()
{
    TimerStart( 6 );
    m_particle_file.setParameterFromFile();
    TimerStop( 6 );
    TimerStart( 7 );
    m_particle.clear();
    m_particle_file.generatePointObject( m_time_step, &m_particle );
    TimerStop( 7 );
}

void ParticleMonitor::readParticleHistoryFile()
{
    std::stringstream step;
    step << '_' << std::setw( 5 ) << std::setfill( '0' ) << m_time_step;
    std::string history_file_name = m_history_file_prefix + step.str() + ".txt";
    this->setParticleHistoryFileName( history_file_name );
    m_history_file.read();
}

void ParticleMonitor::setParticleFilePrefix( const std::string& prefix )
{
    m_particle_file.setFilePrefix( prefix );
}

void ParticleMonitor::setParticleStatusFileName( const std::string& file_name )
{
    m_status_file.setFileName( file_name );
}

void ParticleMonitor::setParticleHistoryFileName( const std::string& file_name )
{
    m_history_file.setFileName( file_name );
}

bool ParticleMonitor::setTimeStep( const kvs::Int32 time_step )
{
    bool changed = ( m_time_step != time_step ) && this->stepExisted();
    m_time_step = time_step;
    return changed;
}

void ParticleMonitor::getParticle( pbvr::PointObject* object )
{
    (*object) = m_particle;
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

bool ParticleMonitor::statusFileChanged()
{
    return !this->checking_status_file();
}

kvs::Int32 ParticleMonitor::getTimeStep()
{
    return m_time_step;
}

ParticleMonitor::~ParticleMonitor()
{
}
