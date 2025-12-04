#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string.h>
#include <map>

#include "GlyphProperty.h"

bool GlyphProperty::LoadIN( const std::string& filename )
{
    name_list.clear();
    param.clear();
    param_state.clear();

    std::ifstream fin( filename.c_str(), std::ios::in );
    std::string name;
    std::string value;
    std::string equal;
    if ( !fin.is_open() )
    {
        std::cerr << "Cannot open " << filename << std::endl;
        fin.close();
        return false;
    }

    std::string line;

    name_list.push_back( "GLYPH_FLAG" );
    name_list.push_back( "DIRECTION_VARIABLES" );
    name_list.push_back( "SIZE_SAMPLING_METHOD" );
    name_list.push_back( "SIZE_VARIABLES" );
    name_list.push_back( "DISTRIBUTION_MODE" );
    name_list.push_back( "STRIDE" );
    name_list.push_back( "SEED" );
    name_list.push_back( "NUMBER_OF_SMAPLING_POINT" );
    name_list.push_back( "GLYPH_COLOR_MAP_TABLE" );
    name_list.push_back( "COLOR_DATA_SAMPLING_METHOD" );
    name_list.push_back( "COLOR_VARIABLES" );
    name_list.push_back( "GLYPH_COLOR_MAX" );
    name_list.push_back( "GLYPH_COLOR_MIN" );
    name_list.push_back( "END_PARAMETER_FILE" );

    for ( std::vector<std::string>::iterator i = name_list.begin(); i != name_list.end(); i++ )
    {
        param_state[*i] = false;
    }

    while ( std::getline( fin, line ) )
    {
        size_t pos;
        while ( ( pos = line.find_first_of( " 　\t\r\n" ) ) != std::string::npos )
        {
            line.erase( pos, 1 );
        }

        if ( ( pos = line.find_first_of( "=" ) ) != std::string::npos )
        {
            name = line.substr( 0, pos );
            value = line.substr( pos + 1 );
        }
        else
        {
            name = "";
        }

        std::string key = "";

        for ( std::vector<std::string>::iterator i = name_list.begin(); i != name_list.end(); i++ )
        {
            if ( *i == name ) key = name;
        }

        if ( key != "" )
        {
            param[key]       = value;
            param_state[key] = true;
        }
    }

    return true;
}

int GlyphProperty::getInt( std::string name )
{
    return std::atoi( param[name].c_str() );
}

float GlyphProperty::getFloat( std::string name )
{
    return std::atof( param[name].c_str() );
}

std::string GlyphProperty::getString( std::string name )
{
    if ( param.find( name ) == param.end() )
        return "";
    else
        return param[name];
}

std::vector<std::string> GlyphProperty::getTableString( std::string name )
{
    std::string list = param[name];
    std::vector<std::string> table;

    while( 1 )
    {
        int pos = list.find_first_of(",");
        if( pos == std::string::npos ) break;
        std::string num = list.substr( 0, pos );
        table.push_back( num.c_str() );
        list = list.substr( pos+1 );
    }

    return table;
}

std::vector<int> GlyphProperty::getTableInt( std::string name )
{
    std::string list = param[name];
    std::vector<int> table;

    while( 1 )
    {
        int pos = list.find_first_of(",");
        if( pos == std::string::npos ) break;
        std::string num = list.substr( 0, pos );
        table.push_back( std::atoi( num.c_str() ) );
        list = list.substr( pos+1 );
    }

    return table;
}

std::vector<float> GlyphProperty::getTableFloat( std::string name )
{
    std::string list = param[name];
    std::vector<float> table;

    while( 1 )
    {
        int pos = list.find_first_of(",");
        if( pos == std::string::npos ) break;
        std::string num = list.substr( 0, pos );
        table.push_back( std::atof( num.c_str() ) );
        list = list.substr( pos+1 );
    }

    return table;
}

void GlyphProperty::write( const std::string name )
{
    std::ofstream ofs;
    ofs.open( name.c_str(), std::ios::out | std::ios::trunc  );

    for ( std::map<std::string, std::string>::iterator i = param.begin(); i != param.end(); i++ )
    {
        ofs << i->first  <<  "=" << i->second << std::endl;
    }

    ofs.close();
}

#if 0
size_t GlyphProperty::byteSize() const
{
    size_t index = 0;
   
    index += jpv::Serializer::byteSize( param.size() );
    for ( std::map<std::string, std::string>::const_iterator i = param.begin(); i != param.end(); i++ )
    {
        index += jpv::Serializer::byteSize( i->first );
        index += jpv::Serializer::byteSize( i->second );
    }

    return index;
}

size_t GlyphProperty::pack( char* buf ) const
{
    size_t index = 0;
   
    index += jpv::Serializer::write( buf + index, param.size() );
    for ( std::map<std::string, std::string>::const_iterator i = param.begin(); i != param.end(); i++ )
    {
        index += jpv::Serializer::write( buf + index, i->first );
        index += jpv::Serializer::write( buf + index, i->second );
    }

    return index;
}

size_t GlyphProperty::unpack( const char* buf )
{
    std::string nm;
    std::string val;
    size_t s;
    size_t index = 0;

    for ( std::vector<std::string>::iterator i = name_list.begin(); i != name_list.end(); i++ )
    {
        param_state[*i] = false;
    }

    index += jpv::Serializer::read( buf + index, s );
    for ( size_t i = 0; i != s; i++ )
    {
        index += jpv::Serializer::read( buf + index, nm );
        index += jpv::Serializer::read( buf + index, val );
        param[nm] = val;
        param_state[nm] = true;
    }

    return index;
}
#endif
