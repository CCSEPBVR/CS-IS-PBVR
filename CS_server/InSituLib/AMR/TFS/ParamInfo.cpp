#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string.h>
#include <map>

#include "ParamInfo.h"

bool ParamInfo::LoadIN( const std::string& filename )
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

    name_list.push_back( "SUB_PIXEL_LEVEL" );
    name_list.push_back( "REPEAT_LEVEL" );
    name_list.push_back( "PARTICLE_DENSITY" );
    name_list.push_back( "PARTICLE_LIMIT" );
    name_list.push_back( "PARTICLE_DATA_SIZE_LIMIT" );
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
    //name_list.push_back( "TF_SYNTH" );
    name_list.push_back( "COLOR_SYNTH" );
    name_list.push_back( "OPACITY_SYNTH" );

    name_list.push_back( "TF_NUMBER" );
#if 0
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
#endif
    //2019 kawamura
    name_list.push_back( "OPA_FUNC_EXP_TOKEN" );// ex) A1+A2*A3
    name_list.push_back( "OPA_FUNC_VAR_NAME" );
    name_list.push_back( "OPA_FUNC_VAL_ARRAY" );
    name_list.push_back( "COL_FUNC_EXP_TOKEN" );// ex) C1-C2/C3
    name_list.push_back( "COL_FUNC_VAR_NAME" );
    name_list.push_back( "COL_FUNC_VAL_ARRAY" );
#if 0
    name_list.push_back( "TF_NAME1_C_EXP_TOKEN" );// ex) Q1*Q2*Q3
    name_list.push_back( "TF_NAME1_C_VAR_NAME" );
    name_list.push_back( "TF_NAME1_C_VAL_ARRAY" );
    name_list.push_back( "TF_NAME1_O_EXP_TOKEN" );
    name_list.push_back( "TF_NAME1_O_VAR_NAME" );
    name_list.push_back( "TF_NAME1_O_VAL_ARRAY" );
    name_list.push_back( "TF_NAME2_C_EXP_TOKEN" );
    name_list.push_back( "TF_NAME2_C_VAR_NAME" );
    name_list.push_back( "TF_NAME2_C_VAL_ARRAY" );
    name_list.push_back( "TF_NAME2_O_EXP_TOKEN" );
    name_list.push_back( "TF_NAME2_O_VAR_NAME" );
    name_list.push_back( "TF_NAME2_O_VAL_ARRAY" );
    name_list.push_back( "TF_NAME3_C_EXP_TOKEN" );
    name_list.push_back( "TF_NAME3_C_VAR_NAME" );
    name_list.push_back( "TF_NAME3_C_VAL_ARRAY" );
    name_list.push_back( "TF_NAME3_O_EXP_TOKEN" );
    name_list.push_back( "TF_NAME3_O_VAR_NAME" );
    name_list.push_back( "TF_NAME3_O_VAL_ARRAY" );
    name_list.push_back( "TF_NAME4_C_EXP_TOKEN" );
    name_list.push_back( "TF_NAME4_C_VAR_NAME" );
    name_list.push_back( "TF_NAME4_C_VAL_ARRAY" );
    name_list.push_back( "TF_NAME4_O_EXP_TOKEN" );
    name_list.push_back( "TF_NAME4_O_VAR_NAME" );
    name_list.push_back( "TF_NAME4_O_VAL_ARRAY" );
    name_list.push_back( "TF_NAME5_C_EXP_TOKEN" );
    name_list.push_back( "TF_NAME5_C_VAR_NAME" );
    name_list.push_back( "TF_NAME5_C_VAL_ARRAY" );
    name_list.push_back( "TF_NAME5_O_EXP_TOKEN" );
    name_list.push_back( "TF_NAME5_O_VAR_NAME" );
    name_list.push_back( "TF_NAME5_O_VAL_ARRAY" );
#endif

    for (size_t n = 0; n < 99; n++) {
        std::stringstream ss;
        ss << "TF_NAME" << n+1 << "_";
        const std::string tag_base = ss.str();

        name_list.push_back( tag_base + "VAR_C" );
        name_list.push_back( tag_base + "MIN_C"   );
        name_list.push_back( tag_base + "MAX_C"   );
        name_list.push_back( tag_base + "VAR_O"   );
        name_list.push_back( tag_base + "MIN_O"   );
        name_list.push_back( tag_base + "MAX_O"   );
        name_list.push_back( tag_base + "TABLE_C" );
        name_list.push_back( tag_base + "TABLE_O" );

        name_list.push_back( tag_base + "C_EXP_TOKEN" );// ex) Q1*Q2*Q3
        name_list.push_back( tag_base + "C_VAR_NAME" );
        name_list.push_back( tag_base + "C_VAL_ARRAY" );
        name_list.push_back( tag_base + "O_EXP_TOKEN" );
        name_list.push_back( tag_base + "O_VAR_NAME" );
        name_list.push_back( tag_base + "O_VAL_ARRAY" );
    }

    name_list.push_back( "SERVER_LATENCY" );
    name_list.push_back( "SERVER_JOB_NUMBER" );
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

    // delete TF_NUMBER ~ 99
    if ( param_state["TF_NUMBER"] == true ) {
        int tf_number = getInt("TF_NUMBER");

        for (size_t n = tf_number; n < 99; n++) {
            std::stringstream ss;
            ss << "TF_NAME" << n+1 << "_";
            const std::string tag_base = ss.str();

            param_state.erase( tag_base + "VAR_C" );
            param_state.erase( tag_base + "MIN_C"   );
            param_state.erase( tag_base + "MAX_C"   );
            param_state.erase( tag_base + "VAR_O"   );
            param_state.erase( tag_base + "MIN_O"   );
            param_state.erase( tag_base + "MAX_O"   );
            param_state.erase( tag_base + "TABLE_C" );
            param_state.erase( tag_base + "TABLE_O" );
            param_state.erase( tag_base + "C_EXP_TOKEN" );
            param_state.erase( tag_base + "C_VAR_NAME" );
            param_state.erase( tag_base + "C_VAL_ARRAY" );
            param_state.erase( tag_base + "O_EXP_TOKEN" );
            param_state.erase( tag_base + "O_VAR_NAME" );
            param_state.erase( tag_base + "O_VAL_ARRAY" );

            param.erase( tag_base + "VAR_C" );
            param.erase( tag_base + "MIN_C"   );
            param.erase( tag_base + "MAX_C"   );
            param.erase( tag_base + "VAR_O"   );
            param.erase( tag_base + "MIN_O"   );
            param.erase( tag_base + "MAX_O"   );
            param.erase( tag_base + "TABLE_C" );
            param.erase( tag_base + "TABLE_O" );
            param.erase( tag_base + "C_EXP_TOKEN" );
            param.erase( tag_base + "C_VAR_NAME" );
            param.erase( tag_base + "C_VAL_ARRAY" );
            param.erase( tag_base + "O_EXP_TOKEN" );
            param.erase( tag_base + "O_VAR_NAME" );
            param.erase( tag_base + "O_VAL_ARRAY" );

            std::vector<std::string>::iterator itr = name_list.begin();
            while (itr != name_list.end()) {
                if ((*itr) == tag_base + "VAR_C"  ||
                    (*itr) == tag_base + "MIN_C"    ||
                    (*itr) == tag_base + "MAX_C"    ||
                    (*itr) == tag_base + "VAR_O"    ||
                    (*itr) == tag_base + "MIN_O"    ||
                    (*itr) == tag_base + "MAX_O"    ||
                    (*itr) == tag_base + "TABLE_C"  ||
                    (*itr) == tag_base + "TABLE_O"  ||
                    (*itr) == tag_base + "C_EXP_TOKEN"  ||
                    (*itr) == tag_base + "C_VAR_NAME"  ||
                    (*itr) == tag_base + "C_VAL_ARRAY"  ||
                    (*itr) == tag_base + "O_EXP_TOKEN"  ||
                    (*itr) == tag_base + "O_VAR_NAME"  ||
                    (*itr) == tag_base + "O_VAL_ARRAY") 
                {
                    itr = name_list.erase(itr);
                }
                else {
                    itr++;
                }
            }
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

std::vector<int> ParamInfo::getTableInt( std::string name )
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

std::vector<float> ParamInfo::getTableFloat( std::string name )
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

void ParamInfo::write( const std::string name )
{
    std::ofstream ofs;
    ofs.open( name.c_str(), std::ios::out | std::ios::trunc  );

    for ( std::map<std::string, std::string>::iterator i = param.begin(); i != param.end(); i++ )
    {
        ofs << i->first  <<  "=" << i->second << std::endl;
    }

    ofs.close();
}

size_t ParamInfo::byteSize() const
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

size_t ParamInfo::pack( char* buf ) const
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

size_t ParamInfo::unpack( const char* buf )
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

