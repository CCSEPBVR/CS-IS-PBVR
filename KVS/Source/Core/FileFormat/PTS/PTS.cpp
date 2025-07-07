#include "PTS.h"
#include <kvs/IgnoreUnusedVariable>
#include <kvs/File>
#include <kvs/Assert>
/****************************************************************************/
/**
 *  @file   PTS.cpp
 *  @author
 */
/****************************************************************************/

namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Checks the file extension.
 *  @param  filename [in] filename
 *  @return true, if the given filename has the supported extension
 */
/*===========================================================================*/
bool PTS::CheckExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "pts" || file.extension() == "PTS" )
    {
        return true;
    }

    return false;
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PTS class.
 */
/*===========================================================================*/
PTS::PTS()
{

}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PTS class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
PTS::PTS( const std::string& filename )
{
    this->read( filename );
}


/*===========================================================================*/
/**
 *  @brief  Read a PTS file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is successfully
 */
/*===========================================================================*/
bool PTS::read( const std::string& filename )
{
    BaseClass::setFilename( filename );
    BaseClass::setSuccess( true );

    std::ifstream ifs( filename, std::ios::in);

    std::string line_buffer;

    // Read first line as num of points.
    std::getline(ifs, line_buffer);    
    m_npoints = std::stoi( line_buffer );    

    // Read second line as data components
    std::getline(ifs, line_buffer);    
    // Count num of components in a line
    std::vector<std::string> data;
    data = this->split( line_buffer, ' ' );
    m_ncomponents = data.size();    

    kvs::ValueArray<kvs::Real32> coordinates( 3*m_npoints );
    kvs::ValueArray<kvs::UInt8>  colors     ( 3*m_npoints );

    if( m_ncomponents == 4 )
    {
        coordinates[0] = std::stof( data[0] );
        coordinates[1] = std::stof( data[1] );
        coordinates[2] = std::stof( data[2] );

        colors[0] = (kvs::UInt8)std::stoi( data[3] );
        colors[1] = (kvs::UInt8)std::stoi( data[3] );
        colors[2] = (kvs::UInt8)std::stoi( data[3] );

        for( int i = 1; i < m_npoints; i++ )
        {
            // Read lines
            std::getline(ifs, line_buffer);
            data = this->split( line_buffer, ' ' );

            coordinates[3*i  ] = std::stof( data[0] );
            coordinates[3*i+1] = std::stof( data[1] );
            coordinates[3*i+2] = std::stof( data[2] );

            colors[3*i  ] = (kvs::UInt8)std::stoi( data[3] );
            colors[3*i+1] = (kvs::UInt8)std::stoi( data[3] );
            colors[3*i+2] = (kvs::UInt8)std::stoi( data[3] );
        }
    }
    else if( m_ncomponents == 6 || m_ncomponents == 9 )
    {
        coordinates[0] = std::stof( data[0] );
        coordinates[1] = std::stof( data[1] );
        coordinates[2] = std::stof( data[2] );

        colors[0] = (kvs::UInt8)std::stoi( data[3] );;
        colors[1] = (kvs::UInt8)std::stoi( data[4] );
        colors[2] = (kvs::UInt8)std::stoi( data[5] );

        for( int i = 1; i < m_npoints; i++ )
        {
            // Read lines
            std::getline(ifs, line_buffer);
            data = this->split( line_buffer, ' ' );

            coordinates[3*i  ] = std::stof( data[0] );
            coordinates[3*i+1] = std::stof( data[1] );
            coordinates[3*i+2] = std::stof( data[2] );

            colors[3*i  ] = (kvs::UInt8)std::stoi( data[3] );
            colors[3*i+1] = (kvs::UInt8)std::stoi( data[4] );
            colors[3*i+2] = (kvs::UInt8)std::stoi( data[5] );
        }
    }
    else if( m_ncomponents == 7 || m_ncomponents == 10 )
    {
        coordinates[0] = std::stof( data[0] );
        coordinates[1] = std::stof( data[1] );
        coordinates[2] = std::stof( data[2] );

        colors[0] = (kvs::UInt8)std::stoi( data[4] );;
        colors[1] = (kvs::UInt8)std::stoi( data[5] );
        colors[2] = (kvs::UInt8)std::stoi( data[6] );

        for( int i = 1; i < m_npoints; i++ )
        {
            // Read lines
            std::getline(ifs, line_buffer);
            data = this->split( line_buffer, ' ' );

            coordinates[3*i  ] = std::stof( data[0] );
            coordinates[3*i+1] = std::stof( data[1] );
            coordinates[3*i+2] = std::stof( data[2] );

            colors[3*i  ] = (kvs::UInt8)std::stoi( data[4] );
            colors[3*i+1] = (kvs::UInt8)std::stoi( data[5] );
            colors[3*i+2] = (kvs::UInt8)std::stoi( data[6] );
        }
    }
    else
    {
        kvsMessageError( "Unsupported PTS Type" );
        ifs.close();
        return false;
    }

    m_coords = kvs::ValueArray<kvs::Real32>( coordinates );
    m_colors = kvs::ValueArray<UInt8>( colors );

    ifs.close();
    return true;
}

/*===========================================================================*/
/**
 *  @brief  Writes PTS data.
 *  @param  filename [in] filename
 *  @return false (not yet implemented)
 */
/*===========================================================================*/
bool PTS::write( const std::string& filename )
{
    kvs::IgnoreUnusedVariable( filename );
    return false;
}

std::vector<std::string> PTS::split(const std::string& input, char delimiter)
{
    std::istringstream stream(input);

    std::string field;
    std::vector<std::string> result;
    while (std::getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
}

} // end of namespace kvs
