#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string.h>
#include <map>

#include "ParamInfo.h"

bool ParamInfo::LoadIN( const std::string& filename )
{
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

    name_list.push_back( "SUB_PIXEL_LEVEL" );
    name_list.push_back( "REPEAT_LEVEL" );
    name_list.push_back( "PARTICLE_LIMIT" );
    name_list.push_back( "RESOLUTION_WIDTH" );
    name_list.push_back( "RESOLUTION_HEIGHT" );
    name_list.push_back( "CROP_TYPE" );
    name_list.push_back( "CROP_CXMIN" );
    name_list.push_back( "CROP_CYMIN" );
    name_list.push_back( "CROP_CZMIN" );
    name_list.push_back( "CROP_CXMAX" );
    name_list.push_back( "CROP_CYMAX" );
    name_list.push_back( "CROP_CZMAX" );
    name_list.push_back( "CROP_SCENTERX" );
    name_list.push_back( "CROP_SCENTERY" );
    name_list.push_back( "CROP_SCENTERZ" );
    name_list.push_back( "CROP_SRADIUS" );
    name_list.push_back( "CROP_PCENTERX" );
    name_list.push_back( "CROP_PCENTERY" );
    name_list.push_back( "CROP_PCENTERZ" );
    name_list.push_back( "CROP_PRADIUS" );
    name_list.push_back( "CROP_PHEIGHT" );
    name_list.push_back( "PFI_PATH_SERVER" );
    name_list.push_back( "TF_RESOLUTION" );
    name_list.push_back( "TF_SYNTH" );
    name_list.push_back( "TF_NAME1_VAR_C" );
    name_list.push_back( "TF_NAME1_MIN_C"   );
    name_list.push_back( "TF_NAME1_MAX_C"   );
    name_list.push_back( "TF_NAME1_VAR_O"   );
    name_list.push_back( "TF_NAME1_MIN_O"   );
    name_list.push_back( "TF_NAME1_MAX_O"   );
    name_list.push_back( "TF_NAME1_TABLE_C" );
    name_list.push_back( "TF_NAME1_TABLE_O" );
    name_list.push_back( "TF_NAME2_VAR_C"   );
    name_list.push_back( "TF_NAME2_MIN_C"   );
    name_list.push_back( "TF_NAME2_MAX_C"   );
    name_list.push_back( "TF_NAME2_VAR_O"   );
    name_list.push_back( "TF_NAME2_MIN_O"   );
    name_list.push_back( "TF_NAME2_MAX_O"   );
    name_list.push_back( "TF_NAME2_TABLE_C" );
    name_list.push_back( "TF_NAME2_TABLE_O" );
    name_list.push_back( "TF_NAME3_VAR_C"   );
    name_list.push_back( "TF_NAME3_MIN_C"   );
    name_list.push_back( "TF_NAME3_MAX_C"   );
    name_list.push_back( "TF_NAME3_VAR_O"   );
    name_list.push_back( "TF_NAME3_MIN_O"   );
    name_list.push_back( "TF_NAME3_MAX_O"   );
    name_list.push_back( "TF_NAME3_TABLE_C" );
    name_list.push_back( "TF_NAME3_TABLE_O" );
    name_list.push_back( "TF_NAME4_VAR_C"   );
    name_list.push_back( "TF_NAME4_MIN_C"   );
    name_list.push_back( "TF_NAME4_MAX_C"   );
    name_list.push_back( "TF_NAME4_VAR_O"   );
    name_list.push_back( "TF_NAME4_MIN_O"   );
    name_list.push_back( "TF_NAME4_MAX_O"   );
    name_list.push_back( "TF_NAME4_TABLE_C" );
    name_list.push_back( "TF_NAME4_TABLE_O" );
    name_list.push_back( "TF_NAME5_VAR_C"   );
    name_list.push_back( "TF_NAME5_MIN_C"   );
    name_list.push_back( "TF_NAME5_MAX_C"   );
    name_list.push_back( "TF_NAME5_VAR_O"   );
    name_list.push_back( "TF_NAME5_MIN_O"   );
    name_list.push_back( "TF_NAME5_MAX_O"   );
    name_list.push_back( "TF_NAME5_TABLE_C" );
    name_list.push_back( "TF_NAME5_TABLE_O" );
    name_list.push_back( "SERVER_LATENCY" );
    name_list.push_back( "SERVER_JOB_NUMBER" );

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

int ParamInfo::getInt( std::string name )
{
    return std::atoi( param[name].c_str() );
}

float ParamInfo::getFloat( std::string name )
{
    return std::atof( param[name].c_str() );
}

std::string ParamInfo::getString( std::string name )
{
    if ( param.find( name ) == param.end() )
        return "";
    else
        return param[name];
}
