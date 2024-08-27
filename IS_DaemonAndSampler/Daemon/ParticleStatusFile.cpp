#include "ParticleStatusFile.h"

ParticleStatusFile::ParticleStatusFile():
    m_start_time_step( 0 ),
    m_latest_time_step( 0 ),
    m_status( NO_STEP )
{
    this->set_name();
}

ParticleStatusFile::ParticleStatusFile( const std::string& file ):
    m_file_name( file ),
    m_start_time_step( 0 ),
    m_latest_time_step( 0 ),
    m_status( NO_STEP )
{
    this->set_name();
}

void ParticleStatusFile::set_name()
{
    m_name.push_back( "START_STEP" );
    m_name.push_back( "LATEST_STEP" );
}

void ParticleStatusFile::assign_name_list( const NameListFile& name_list_file )
{
    NameListFile nml = name_list_file;

    m_start_time_step  = nml.getValue<kvs::UInt32>( "START_STEP" );
    m_latest_time_step = nml.getValue<kvs::UInt32>( "LATEST_STEP" );
}

void ParticleStatusFile::read_name_list_file()
{
    const Status back_status = m_status;
    NameListFile nlfile( m_file_name );

    for( NameArray::iterator i = m_name.begin(); i != m_name.end(); i++ )
    {
        nlfile.setName( *i );
    }

    if( nlfile.read() )
    {
        if( nlfile.getValue<std::string>( "LATEST_STEP" ) != "NO_STEP"
         && nlfile.getValue<std::string>( "LATEST_STEP" ) != "" )
        {
            const kvs::UInt32 back_time_step = m_latest_time_step;

            this->assign_name_list( nlfile );
            if ( back_status != NO_STEP )
            {
                if( back_time_step != m_latest_time_step )
                {
                    m_status = STEP_CHANGED;
                }
                else
                {
                    m_status = SAME_STEP;
                }
            }
            else
            {
                m_status = STEP_CHANGED;
            }
        }
        else
        {
            m_status = NO_STEP;
        } 
    }
    else
    {
        m_status = NO_STATUS_FILE;
    }
}

void ParticleStatusFile::setFileName( const std::string& file )
{
    m_file_name = file;
}

void ParticleStatusFile::read()
{
    this->read_name_list_file();
}

kvs::UInt32 ParticleStatusFile::getLatestTimeStep()
{
    return m_latest_time_step;
}

kvs::UInt32 ParticleStatusFile::getStartTimeStep()
{
    return m_start_time_step;
}

ParticleStatusFile::Status ParticleStatusFile::getStatus()
{
   return m_status;
}
