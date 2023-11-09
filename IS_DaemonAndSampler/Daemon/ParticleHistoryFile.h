#ifndef __JUPITER_PARTICLE_HISTORY_FILE_H__
#define __JUPITER_PARTICLE_HISTORY_FILE_H__

#include <iostream>
#include <string>
#include <vector>
#include <kvs/Type>
#include "NameListFile.h"
#include "../Common/VariableRange.h" 

class ParticleHistoryFile
{
private:
    typedef std::vector<std::string> NameArray;
    typedef std::vector< std::vector<int> > HistogramArray;
   
    std::string       m_file_name;
    NameArray         m_name;
    VariableRange     m_variable_range;
    HistogramArray    m_color_histogram_array;
    HistogramArray    m_opacity_histogram_array;

private:

    template<typename T>
    inline T from_string( const std::string& name );

    template<typename T>
    inline std::template vector<T> split( const std::string& line, const char delim  );

    template<typename T>
    inline std::template vector<T> split_csv( const std::string& line );

    void set_name();
    void assign_name_list( const NameListFile& name_list_file );
    void read_name_list_file();

public:
    ParticleHistoryFile();
    ParticleHistoryFile( const std::string& file );
    void setFileName( const std::string& file );
    void read();
    VariableRange& variableRange();
    HistogramArray& colorHistogramArray();
    HistogramArray& opacityHistogramArray();
};

template<typename T>
inline T ParticleHistoryFile::from_string( const std::string& name )
{
    T value;
    std::istringstream is( name );
    is >> value;
    return value;
}

template<typename T>
inline std::template vector<T> ParticleHistoryFile::split( const std::string& line, const char delim  )
{
    std::template vector<T> array;
    std::stringstream ss_line( line );
    std::string item;

    while ( std::getline( ss_line, item, delim ) )
    {
        array.push_back( this->from_string<T>( item ) );
    }

    return array;
}

template<typename T>
inline std::template vector<T> ParticleHistoryFile::split_csv( const std::string& line )
{
    return split<T>( line, ',' );
}
#endif
