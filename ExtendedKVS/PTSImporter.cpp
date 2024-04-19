#include "PTSImporter.h"

PTSImporter::PTSImporter( std::string filename )
{
    std::ifstream ifs( filename, std::ios::in);

    std::string line_buffer;

    // Read first line as num of points.
    std::getline(ifs, line_buffer);
    std::cout << line_buffer << std::endl;
    m_npoints = std::stoi( line_buffer );
    std::cout << "npoints = " << m_npoints << std::endl;

    // Read second line as data components
    std::getline(ifs, line_buffer);
    std::cout << line_buffer << std::endl;
    // Count num of components in a line
    std::vector<std::string> data;
    data = this->split( line_buffer, ' ' );
    m_ncomponents = data.size();
    std::cout << "ncomponents = " << m_ncomponents << ": ";

    if( m_ncomponents == 4 )
    {
        std::cout << "X Y Z I" << std::endl;
    }
    else if( m_ncomponents == 6 )
    {
        std::cout << "X Y Z R G B" << std::endl;
    }
    else if( m_ncomponents == 7 )
    {
        std::cout << "X Y Z I R G B" << std::endl;
    }
    else if( m_ncomponents == 9 )
    {
        std::cout << "X Y Z R G B nX nY nZ" << std::endl;
    }
    else if( m_ncomponents == 10 )
    {
        std::cout << "X Y Z I R G B nX nY nZ" << std::endl;
    }
    else
    {
        std::cout << "Unsupported PTS Type" << std::endl;
    }


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
        ifs.close();
        return;
    }

    this->setCoords ( coordinates );
    this->setColors ( colors );
    this->updateMinMaxCoords();

    ifs.close();
}

std::vector<std::string> PTSImporter::split(const std::string& input, char delimiter)
{
    std::istringstream stream(input);

    std::string field;
    std::vector<std::string> result;
    while (std::getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
}
