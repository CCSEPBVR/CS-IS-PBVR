#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string.h>
#include <map>

#include "ParameterFile.h"

bool ParameterFile::LoadIN( const std::string& filename )
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
    const std::string TF_NAME = "TF_NAME";

    m_name_list.push_back( "SUB_PIXEL_LEVEL" );
    m_name_list.push_back( "REPEAT_LEVEL" );
    m_name_list.push_back( "PARTICLE_LIMIT" );
    m_name_list.push_back( "RESOLUTION_WIDTH" );
    m_name_list.push_back( "RESOLUTION_HEIGHT" );
    m_name_list.push_back( "CROP_TYPE" );
    m_name_list.push_back( "CROP_CXMIN" );
    m_name_list.push_back( "CROP_CYMIN" );
    m_name_list.push_back( "CROP_CZMIN" );
    m_name_list.push_back( "CROP_CXMAX" );
    m_name_list.push_back( "CROP_CYMAX" );
    m_name_list.push_back( "CROP_CZMAX" );
    m_name_list.push_back( "CROP_SCENTERX" );
    m_name_list.push_back( "CROP_SCENTERY" );
    m_name_list.push_back( "CROP_SCENTERZ" );
    m_name_list.push_back( "CROP_SRADIUS" );
    m_name_list.push_back( "CROP_PCENTERX" );
    m_name_list.push_back( "CROP_PCENTERY" );
    m_name_list.push_back( "CROP_PCENTERZ" );
    m_name_list.push_back( "CROP_PRADIUS" );
    m_name_list.push_back( "CROP_PHEIGHT" );
    m_name_list.push_back( "PFI_PATH_SERVER" );
    m_name_list.push_back( "TF_RESOLUTION" );
    // add by @hira at 2016/12/01
    m_name_list.push_back( "TF_NUMBER" );
    m_name_list.push_back( "TF_SYNTH_C" );
    m_name_list.push_back( "TF_SYNTH_O" );
    // add by @shimomura 2022/12/5
    m_name_list.push_back( "COLOR_SYNTH" );
    m_name_list.push_back( "OPACITY_SYNTH" );
#if 0		// delete by @hira at 2016/12/01
    m_name_list.push_back( "TF_SYNTH" );
    m_name_list.push_back( "TF_NAME1_VAR_C" );
    m_name_list.push_back( "TF_NAME1_MIN_C"   );
    m_name_list.push_back( "TF_NAME1_MAX_C"   );
    m_name_list.push_back( "TF_NAME1_VAR_O"   );
    m_name_list.push_back( "TF_NAME1_MIN_O"   );
    m_name_list.push_back( "TF_NAME1_MAX_O"   );
    m_name_list.push_back( "TF_NAME1_TABLE_C" );
    m_name_list.push_back( "TF_NAME1_TABLE_O" );
    m_name_list.push_back( "TF_NAME2_VAR_C"   );
    m_name_list.push_back( "TF_NAME2_MIN_C"   );
    m_name_list.push_back( "TF_NAME2_MAX_C"   );
    m_name_list.push_back( "TF_NAME2_VAR_O"   );
    m_name_list.push_back( "TF_NAME2_MIN_O"   );
    m_name_list.push_back( "TF_NAME2_MAX_O"   );
    m_name_list.push_back( "TF_NAME2_TABLE_C" );
    m_name_list.push_back( "TF_NAME2_TABLE_O" );
    m_name_list.push_back( "TF_NAME3_VAR_C"   );
    m_name_list.push_back( "TF_NAME3_MIN_C"   );
    m_name_list.push_back( "TF_NAME3_MAX_C"   );
    m_name_list.push_back( "TF_NAME3_VAR_O"   );
    m_name_list.push_back( "TF_NAME3_MIN_O"   );
    m_name_list.push_back( "TF_NAME3_MAX_O"   );
    m_name_list.push_back( "TF_NAME3_TABLE_C" );
    m_name_list.push_back( "TF_NAME3_TABLE_O" );
    m_name_list.push_back( "TF_NAME4_VAR_C"   );
    m_name_list.push_back( "TF_NAME4_MIN_C"   );
    m_name_list.push_back( "TF_NAME4_MAX_C"   );
    m_name_list.push_back( "TF_NAME4_VAR_O"   );
    m_name_list.push_back( "TF_NAME4_MIN_O"   );
    m_name_list.push_back( "TF_NAME4_MAX_O"   );
    m_name_list.push_back( "TF_NAME4_TABLE_C" );
    m_name_list.push_back( "TF_NAME4_TABLE_O" );
    m_name_list.push_back( "TF_NAME5_VAR_C"   );
    m_name_list.push_back( "TF_NAME5_MIN_C"   );
    m_name_list.push_back( "TF_NAME5_MAX_C"   );
    m_name_list.push_back( "TF_NAME5_VAR_O"   );
    m_name_list.push_back( "TF_NAME5_MIN_O"   );
    m_name_list.push_back( "TF_NAME5_MAX_O"   );
    m_name_list.push_back( "TF_NAME5_TABLE_C" );
    m_name_list.push_back( "TF_NAME5_TABLE_O" );
#endif
    m_name_list.push_back( "SERVER_LATENCY" );
    m_name_list.push_back( "SERVER_JOB_NUMBER" );
    m_name_list.push_back( "COORD1_SYNTH" );
    m_name_list.push_back( "COORD2_SYNTH" );
    m_name_list.push_back( "COORD3_SYNTH" );

    for ( std::vector<std::string>::iterator i = m_name_list.begin(); i != m_name_list.end(); i++ )
    {
        // m_parameter_state[*i] = false;
        if (m_parameter_state.find(*i) != m_parameter_state.end()) m_parameter_state.erase(*i);
        m_parameter_state.insert(std::pair<std::string, bool>(*i, false));
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

        for ( std::vector<std::string>::iterator i = m_name_list.begin(); i != m_name_list.end(); i++ )
        {
            if ( *i == name ) {
                key = name;
                break;
            }
        }

        if (name.find(TF_NAME) == 0) {
            key = name;
        }

        if ( key != "" )
        {
            // m_parameter[key]       = value;
            // m_parameter_state[key] = true;
            if (m_parameter.find(key) != m_parameter.end()) m_parameter.erase(key);
            m_parameter.insert(std::pair<std::string, std::string>(key, value));
            if (m_parameter_state.find(key) != m_parameter_state.end()) m_parameter_state.erase(key);
            m_parameter_state.insert(std::pair<std::string, bool>(key, true));
        }
    }

    return true;
}

int ParameterFile::getInt( const std::string& name )
{
    // return std::atoi( m_parameter[name].c_str() );
    std::string value = m_parameter.at(name);
    return std::atoi( value.c_str());
}

float ParameterFile::getFloat( const std::string& name )
{
    // return std::atof( m_parameter[name].c_str() );
    std::string value = m_parameter.at(name);
    return std::atof( value.c_str());
}

std::string ParameterFile::getString( const std::string& name )
{
    if ( m_parameter.find( name ) == m_parameter.end() )
        return "";
    else {
        // return m_parameter[name];
        std::string value = m_parameter.at(name);
        return value;
    }
}

/**
 * 可視化パラメータの項目数を取得する
 * @return		可視化パラメータの項目数
 */
int ParameterFile::getSize()
{
    return m_parameter.size();
}

/**
 * TF_NAME[NN]_XXXXの項目数を取得する
 * TF_NAME[NN]_VAR_Cの項目を検索する
 * @return		TF_NAME[NN]_XXXXの項目数：NNの数
 */
int ParameterFile::getTfnameSize()
{
    int count = 0;
    std::map<std::string, std::string>::const_iterator itr;
    for (itr=this->m_parameter.begin(); itr!=m_parameter.end(); itr++) {
        std::string item = itr->first;
        std::string value = itr->second;
        if (this->isTfnameNvalc(item)) count++;
    }
    return count;
}

/**
 * TF_NAME[NN]_XXXXの接頭文字列（TF_NAME[NN]）を取得する
 * @param index       インデックス（NNではない）
 * @return		TF_NAME[NN]
 */
std::string ParameterFile::getTfnamePrefix(int index)
{
    int count = 0;
    std::string prefix = "";
    std::map<std::string, std::string>::const_iterator itr;
    for (itr=this->m_parameter.begin(); itr!=m_parameter.end(); itr++) {
        std::string item = itr->first;
        std::string value = itr->first;
        if (!this->isTfnameNvalc(item)) continue;
        if (index == count) {
            int pos = item.find("_VAR_C");
            prefix = item.erase(pos);
            return prefix;
        }
        count++;
    }

    return prefix;
}


/**
 * TF_NAME[NN]_XXXXの番号（TF_NAME[NN]のNN）を取得する
 * @param index       インデックス（NNではない）
 * @return		NN
 */
int ParameterFile::getTfnameNumber(int index)
{
    int count = 0;
    std::string str_num = "";
    int num = 0;
    const std::string TF_NAME = "TF_NAME";
    std::map<std::string, std::string>::const_iterator itr;
    for (itr=this->m_parameter.begin(); itr!=m_parameter.end(); itr++) {
        std::string item = itr->first;
        std::string value = itr->first;
        if (!this->isTfnameNvalc(item)) continue;
        if (index == count) {
            int pos = item.find("_VAR_C");
            str_num = item.erase(pos);
            str_num = str_num.substr(TF_NAME.length());
            return std::atoi(str_num.c_str());
        }
        count++;
    }

    return -1;
}

// add by shimomura 2022/12/06
std::vector<int> ParameterFile::getTableInt( std::string name )
{   
    std::string list = m_parameter[name];
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

std::vector<float> ParameterFile::getTableFloat( std::string name )
{   
    std::string list = m_parameter[name];
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


/**
 * TF_NAME[NN]_VAR_Cの項目であるかチェックする
 * @return		true=TF_NAME[NN]_VAR_C
 */
bool ParameterFile::isTfnameNvalc(const std::string& item)
{
    if (item.find("TF_NAME") == std::string::npos) return false;
    if (item.find("TF_NAME") != 0) return false;
    if (item.find("_VAR_C") == std::string::npos) return false;

    return true;

}
