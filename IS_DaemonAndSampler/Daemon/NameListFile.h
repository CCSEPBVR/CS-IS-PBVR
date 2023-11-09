#ifndef __NAME_LIST_FILE_H__
#define __NAME_LIST_FILE_H__

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <map>

class NameListFile
{
private:

    typedef std::map<std::string, std::string> NameListMap;
  
    std::string m_file_name;
    NameListMap m_name_list;

    template <typename T>
    inline std::string to_string( const T & data );

public:

    NameListFile();
    NameListFile( const std::string& file_name );
    ~NameListFile();

    inline void setName( const std::string& name );

    template <typename T>
    inline void setLine( const std::string& name, const T& value );
    inline void deleteLine( const std::string& name );

    inline void setFileName( const std::string& file_name );

    template<typename T>
    inline T getValue( const std::string& name );
    inline bool getCount( const std::string& name );

    inline void write();
    inline bool read();

    inline bool operator==( NameListFile& name_list_file );
    inline bool operator!=( NameListFile& name_list_file );

};

template <typename T>
inline std::string NameListFile::to_string( const T & data )
{
    std::ostringstream stream;
    stream << data; 
    return( stream.str() );
}

inline NameListFile::NameListFile()
{
}

inline NameListFile::NameListFile( const std::string& file_name )
{
    this->setFileName( file_name );
}

inline void NameListFile::setName( const std::string& name )
{
    m_name_list[ name ] = "";
}

template <typename T>
inline void NameListFile::setLine( const std::string& name, const T & value )
{
    m_name_list[ name ] = to_string( value );
}

inline void NameListFile::deleteLine( const std::string& name )
{
    m_name_list.erase( name );
}

inline void NameListFile::setFileName( const std::string& file_name )
{
    m_file_name = file_name;
}

inline void NameListFile::write()
{
    std::ofstream ofs;
    ofs.open( m_file_name.c_str(), std::ios::out | std::ios::trunc  );

    for ( NameListMap::iterator i = m_name_list.begin(); i != m_name_list.end(); i++ )
    {
        ofs << i->first  <<  "=" << i->second << std::endl;
    }

    ofs.close();
}

inline bool NameListFile::read()
{
    std::ifstream ifs;
    ifs.open( m_file_name.c_str(), std::ios::in );
    //if ( !ifs ) return false;
    if (ifs.is_open() == false) return false;

    std::string line;
    std::string name;
    std::string value;

    while ( std::getline( ifs, line ) )
    {
        size_t pos;
        while ( ( pos = line.find_first_of( " 　\t\r\n" ) ) != std::string::npos )
        {
            line.erase( pos, 1 );
        }

        if ( ( pos = line.find_first_of( "=" ) ) != std::string::npos )
        {
            name  = line.substr( 0, pos );
            value = line.substr( pos + 1 );
        }
        else
        {
            name  = "";
            value = "";
        }

        for ( NameListMap::iterator i = m_name_list.begin(); i != m_name_list.end(); i++ )
        {
            if ( i->first == name )
            {
                i->second = value;
            }
        }
    }

    ifs.close();
    return true;
}

template<typename T>
inline T NameListFile::getValue( const std::string& name )
{
    T value;
    std::istringstream is( m_name_list[name] );
    is >> value;
    return value;
}

inline bool NameListFile::getCount( const std::string& name )
{
    const int count = m_name_list.count( name );
    if (count == 0) return false;
    return true;
}

inline bool NameListFile::operator==( NameListFile& name_list_file )
{
    for( NameListMap::iterator i =  m_name_list.begin(); i != m_name_list.end(); i++ )
    {
        if ( name_list_file.getValue<std::string>( i->first ) != i->second )
            return(false);
    }
    return true ;
}

inline bool NameListFile::operator!=( NameListFile& name_list_file )
{
    return !( *this == name_list_file );
}

inline NameListFile::~NameListFile()
{
}

#endif
